
/*!
	\file

	\author Igor Mironchik (igor.mironchik at gmail dot com).

	Copyright (c) 2023 Igor Mironchik

	Permission is hereby granted, free of charge, to any person
	obtaining a copy of this software and associated documentation
	files (the "Software"), to deal in the Software without
	restriction, including without limitation the rights to use,
	copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the
	Software is furnished to do so, subject to the following
	conditions:

	The above copyright notice and this permission notice shall be
	included in all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
	OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
	NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
	OTHER DEALINGS IN THE SOFTWARE.
*/

// qgiflib include.
#include "qgiflib.hpp"

// C++ include.
#include <vector>

// Qt include.
#include <QPainter>


namespace QGifLib {

//
// Gif
//

bool
Gif::closeHandle( GifFileType * handle )
{
	if( !DGifCloseFile( handle, nullptr ) )
	{
		clean();

		return false;
	}
	else
		return true;
}

bool
Gif::closeHandleWithError( GifFileType * handle )
{
	closeHandle( handle );

	clean();

	return false;
}

bool
Gif::load( const QString & fileName )
{
	clean();

	auto handle = DGifOpenFileName( fileName.toLocal8Bit().data(), nullptr );

	if( handle )
	{
		int animDelay = -1;
		int disposalMode = -1;
		GifRecordType recordType;
		QImage key;

		do
		{
			if( DGifGetRecordType( handle, &recordType ) == GIF_ERROR )
				return closeHandleWithError( handle );

			switch( recordType )
			{
				case IMAGE_DESC_RECORD_TYPE :
				{
					if( DGifGetImageDesc( handle ) == GIF_ERROR )
						return closeHandleWithError( handle );

					int topRow = handle->Image.Top;
					int leftCol = handle->Image.Left;
					int width = handle->Image.Width;
					int height = handle->Image.Height;

					if( width <= 0 || height <= 0 || width > ( INT_MAX / height ) ||
						leftCol + width > handle->SWidth ||
						topRow + height > handle->SHeight )
							return closeHandleWithError( handle );

					QImage img( width, height, QImage::Format_Indexed8 );
					img.fill( handle->SBackGroundColor );

					if( handle->Image.Interlace )
					{
						int InterlacedOffset[] = { 0, 4, 2, 1 };
						int InterlacedJumps[] = { 8, 8, 4, 2 };

						for( int i = 0; i < 4; ++i )
						{
							for( int row = topRow + InterlacedOffset[ i ]; row < topRow + height;
								row += InterlacedJumps[ i ] )
							{
								if( DGifGetLine( handle, img.scanLine( row ), width ) == GIF_ERROR )
									return closeHandleWithError( handle );
							}
						}
					}
					else
					{
						for( int row = 0; row < height; ++row )
						{
							if( DGifGetLine( handle, img.scanLine( row ), width ) == GIF_ERROR )
								return closeHandleWithError( handle );
						}
					}

					++framesCount;

					ColorMapObject * cm = ( handle->Image.ColorMap ? handle->Image.ColorMap :
						handle->SColorMap );

					if( !cm )
						return closeHandleWithError( handle );

					img.setColorCount( cm->ColorCount );

					for( int i = 0; i < cm->ColorCount; ++i )
					{
						GifColorType gifColor = cm->Colors[ i ];
						QRgb color = gifColor.Blue | ( gifColor.Green << 8 ) |
							( gifColor.Red << 16 ) | ( 0xFF << 24 );

						img.setColor( i, color );
					}

					if( key.isNull() )
						key = img.convertedTo( QImage::Format_ARGB32 );
					else
					{
						QImage tmp = key;

						{
							QPainter p( &tmp );
							p.drawImage( leftCol, topRow, img );
						}

						img = tmp;

						if( disposalMode != DISPOSE_PREVIOUS )
							key = img;
					}

					delays.push_back( animDelay );

					if( dir.isValid() )
						img.save( dir.filePath( QString( "%1.png" ).arg( framesCount ) ) );
				}
					break;

				case EXTENSION_RECORD_TYPE :
				{
					GifByteType * extData;
					int extFunction;

					if( DGifGetExtension( handle, &extFunction, &extData ) == GIF_ERROR )
						return closeHandleWithError( handle );

					while( extData != NULL )
					{
						switch( extFunction )
						{
							case GRAPHICS_EXT_FUNC_CODE :
							{
								GraphicsControlBlock b;
								DGifExtensionToGCB( extData[ 0 ], extData + 1, &b );
								animDelay = b.DelayTime * 10;
								disposalMode = b.DisposalMode;
							}
								break;

							default :
								break;
						}

						if( DGifGetExtensionNext( handle, &extData ) == GIF_ERROR )
							return closeHandleWithError( handle );
					}
				}
					break;

				case TERMINATE_RECORD_TYPE :
					break;

				default:
					break;
			}
		} while( recordType != TERMINATE_RECORD_TYPE );

		return closeHandle( handle );
	}
	else
		return false;
}

qsizetype
Gif::count() const
{
	return framesCount;
}

int
Gif::delay( qsizetype idx ) const
{
	return delays.at( idx );
}

QImage
Gif::at( qsizetype idx ) const
{
	if( dir.isValid() )
		return QImage( dir.filePath( QString( "%1.png" ).arg( idx + 1 ) ) );
	else
		return {};
}

bool
Gif::write( const QString & fileName,
	const QStringList & pngFileNames,
	const QVector< int > & delays )
{
	Q_ASSERT( pngFileNames.size() == delays.size() );

	return true;
}

void
Gif::clean()
{
	framesCount = 0;
	delays.clear();
	dir.remove();
	dir = QTemporaryDir( "./" );
}

} /* namespace QGifLib */
