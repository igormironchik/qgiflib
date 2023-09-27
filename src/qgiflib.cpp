
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
#include <memory>

// Qt include.
#include <QPainter>

// Magick++ include.
#include <Magick++.h>


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
Gif::closeEHandle( GifFileType * handle )
{
	if( !EGifCloseFile( handle, nullptr ) )
		return false;
	else
		return true;
}

bool
Gif::closeEHandleWithError( GifFileType * handle )
{
	closeEHandle( handle );

	return false;
}

QStringList
Gif::fileNames() const
{
	QStringList res;

	for( int i = 1; i <= count(); ++i )
		res.push_back( m_dir.filePath( QString( "%1.png" ).arg( i ) ) );

	return res;
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
		int transparentIndex = -1;
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

					++m_framesCount;

					ColorMapObject * cm = ( handle->Image.ColorMap ? handle->Image.ColorMap :
						handle->SColorMap );

					if( !cm )
						return closeHandleWithError( handle );

					img.setColorCount( cm->ColorCount );

					for( int i = 0; i < cm->ColorCount; ++i )
					{
						GifColorType gifColor = cm->Colors[ i ];
						QRgb color = gifColor.Blue | ( gifColor.Green << 8 ) |
							( gifColor.Red << 16 );

						if( i != transparentIndex )
							color |= ( 0xFF << 24 );

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

					m_delays.push_back( animDelay );

					if( m_dir.isValid() )
						img.save( m_dir.filePath( QString( "%1.png" ).arg( m_framesCount ) ) );
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
								transparentIndex = b.TransparentColor;
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
	return m_framesCount;
}

int
Gif::delay( qsizetype idx ) const
{
	return m_delays.at( idx );
}

const QVector< int > &
Gif::delays() const
{
	return m_delays;
}

QImage
Gif::at( qsizetype idx ) const
{
	if( m_dir.isValid() )
		return QImage( m_dir.filePath( QString( "%1.png" ).arg( idx + 1 ) ) );
	else
		return {};
}

namespace {

template< typename T >
struct ArrayDeleter {
	void operator () ( T * p )
	{
		delete [] p;
	}
};

Magick::Image convert( const QImage & qimg )
{
	Magick::Image img( Magick::Geometry( qimg.width(), qimg.height() ),
		Magick::ColorRGB( 0.0, 0.0, 0.0) );

	const double scale = 1.0 / 256.0;

	img.modifyImage();

	Magick::PixelPacket * pixels;
	Magick::ColorRGB mgc;

	for( int y = 0; y < qimg.height(); ++y )
	{
		pixels = img.setPixels( 0, y, img.columns(), 1 );

		for( int x = 0; x < qimg.width(); ++x )
		{
			QColor pix = qimg.pixel( x, y );

			mgc.red( scale * pix.red() );
			mgc.green( scale * pix.green() );
			mgc.blue( scale * pix.blue() );

			*pixels++ = mgc;
		}

		img.syncPixels();
	}

	return img;
}

struct Resources {
	std::shared_ptr< ColorMapObject > cmap;
	std::shared_ptr< GifColorType > colors;
	std::shared_ptr< GifPixelType > pixels;
	int colorMapSize = 0;

	static int color( const Magick::ColorRGB & c )
	{
		const int r = c.red() * 255;
		const int g = c.green() * 255;
		const int b = c.blue() * 255;

		return ( r << 16 ) | ( g << 8 ) | b;
	}

	static int colorMapSizePower2( int s )
	{
		int res = 2;

		while( s > res && res < 256 )
			res = res << 1;

		return res;
	}

	void init( const QImage & img )
	{
		auto tmp = convert( img );
		tmp.quantizeColors( 256 );
		tmp.quantize();

		colorMapSize = colorMapSizePower2( tmp.colorMapSize() );

		cmap = std::make_shared< ColorMapObject > ();
		cmap->ColorCount = colorMapSize;
		cmap->BitsPerPixel = 8;

		colors = std::shared_ptr< GifColorType > ( new GifColorType[ colorMapSize ],
			ArrayDeleter< GifColorType > () );

		cmap->Colors = colors.get();

		int c = 0;

		std::map< int, unsigned char > ccm;

		for( ; c < tmp.colorMapSize(); ++c )
		{
			const auto cc = tmp.colorMap( c );

			colors.get()[ c ].Red = Magick::Color::scaleQuantumToDouble( cc.redQuantum() ) * 255;
			colors.get()[ c ].Green = Magick::Color::scaleQuantumToDouble( cc.greenQuantum() ) * 255;
			colors.get()[ c ].Blue = Magick::Color::scaleQuantumToDouble( cc.blueQuantum() ) * 255;

			ccm[ color( cc ) ] = static_cast< unsigned char > ( c );
		}

		for( ; c < colorMapSize; ++c )
		{
			colors.get()[ c ].Red = 0;
			colors.get()[ c ].Green = 0;
			colors.get()[ c ].Blue = 0;
		}

		pixels = std::shared_ptr< GifPixelType > ( new GifPixelType[ img.width() * img.height() ],
			ArrayDeleter< GifPixelType > () );

		const Magick::PixelPacket * p;
		Magick::ColorRGB mgc;

		for( int y = 0; y < img.height(); ++y)
		{
			p = tmp.getConstPixels( 0, y, img.width(), 1 );

			for( int x = 0; x < img.width(); ++x )
			{
				mgc = *p++;

				pixels.get()[ img.width() * y + x ] = ccm[ color( mgc ) ];
			}
		}
	}
};

inline QImage
loadImage( const QString & fileName )
{
	QImage ret( fileName );

	return ret;
}

bool
addFrame( GifFileType * handle, const QImage & img, const QRect & r, int delay,
	std::vector< Resources > & resources )
{
	GraphicsControlBlock b;
	b.DelayTime = delay / 10;
	b.DisposalMode = DISPOSE_DO_NOT;
	b.TransparentColor = -1;
	b.UserInputFlag = false;

	GifByteType ext[ 4 ];

	const auto len = EGifGCBToExtension( &b, ext );

	if( EGifPutExtension( handle, GRAPHICS_EXT_FUNC_CODE, len, ext ) == GIF_ERROR )
		return false;

	Resources res;
	res.init( img );
	resources.push_back( res );

	if( EGifPutImageDesc( handle, r.x(), r.y(), r.width(), r.height(),
		false, res.cmap.get() ) == GIF_ERROR )
			return false;

	if( EGifPutLine( handle, res.pixels.get(), img.width() * img.height() ) == GIF_ERROR )
		return false;

	return true;
}

std::pair< QImage, QRect >
diffImage( const QImage & key, const QImage & img )
{
	int x = 0;
	int y = 0;
	int width = 0;
	int height = 0;

	int i = 0;
	int j = 0;
	bool different = false;

	for( ; i < key.width(); ++i )
	{
		for( j = 0; j < key.height(); ++j )
		{
			if( key.pixel( i, j ) != img.pixel( i, j ) )
			{
				different = true;

				break;
			}
		}

		if( different )
			break;
	}

	if( i < key.width() )
		x = i;
	else
		return { {}, QRect( 0, 0, 0, 0 ) };

	different = false;

	for( j = 0; j < key.height(); ++j )
	{
		for( i = x; i < key.width(); ++i )
		{
			if( key.pixel( i, j ) != img.pixel( i, j ) )
			{
				different = true;

				break;
			}
		}

		if( different )
			break;
	}

	y = j;

	different = false;

	for( i = key.width() - 1; i > x; --i )
	{
		for( j = y + 1; j < key.height(); ++j )
		{
			if( key.pixel( i, j ) != img.pixel( i, j ) )
			{
				different = true;

				break;
			}
		}

		if( different )
			break;
	}

	width = i - x + 1;

	different = false;

	for( j = key.height() - 1; j > y; --j )
	{
		for( i = x; i < x + width; ++i )
		{
			if( key.pixel( i, j ) != img.pixel( i, j ) )
			{
				different = true;

				break;
			}
		}

		if( different )
			break;
	}

	height = j - y + 1;

	const auto r = QRect( x, y, width, height );

	return { img.copy( r ), r };
}

std::pair< bool, int >
addFrame( GifFileType * handle, QImage & key, const QImage & frame, int delay,
	std::vector< Resources > & resources )
{
	QImage img = frame;

	if( key.width() != img.width() || key.height() != img.height() )
	{
		img = QImage( key.width(), key.height(), QImage::Format_ARGB32 );
		img.fill( Qt::black );

		QPainter p( &img );
		p.drawImage( frame.width() < key.width() ? ( key.width() - frame.width() ) / 2 : 0,
			frame.height() < key.height() ? ( key.height() - frame.height() ) / 2 : 0,
			frame );
	}

	QImage tmp;
	QRect r;

	std::tie( tmp, r ) = diffImage( key, img );

	bool ret = true;
	int delta = delay;

	if( r.width() && r.height() )
	{
		ret = addFrame( handle, tmp, r, delay, resources );

		delta = 0;

		key = img;
	}

	return std::make_pair( ret, delta );
}

} /* namespace */

bool
Gif::write( const QString & fileName,
	const QStringList & pngFileNames,
	const QVector< int > & delays,
	unsigned int loopCount )
{
	if( !pngFileNames.isEmpty() && pngFileNames.size() == delays.size() )
	{
		auto handle = EGifOpenFileName( fileName.toLocal8Bit().data(), false, nullptr );

		if( handle )
		{
			EGifSetGifVersion( handle, true );

			QImage key = loadImage( pngFileNames.front() );

			Resources res;
			res.init( key );

			std::vector< Resources > resources;

			if( EGifPutScreenDesc( handle, key.width(), key.height(), res.colorMapSize, 0,
				res.cmap.get() ) == GIF_ERROR )
					return closeEHandleWithError( handle );

			unsigned char params[ 3 ] = { 1, 0, 0 };
			params[ 1 ] = ( loopCount & 0xFF );
			params[ 2 ] = ( loopCount >> 8 ) & 0xFF;

			if( EGifPutExtensionLeader( handle, APPLICATION_EXT_FUNC_CODE ) == GIF_ERROR )
				return closeEHandleWithError( handle );

			if( EGifPutExtensionBlock( handle, 11, (GifByteType *) "NETSCAPE2.0" ) == GIF_ERROR )
				return closeEHandleWithError( handle );

			if( EGifPutExtensionBlock( handle, sizeof( params ), params ) == GIF_ERROR )
				return closeEHandleWithError( handle );

			if( EGifPutExtensionTrailer( handle ) == GIF_ERROR )
				return closeEHandleWithError( handle );

			if( !addFrame( handle, key, key.rect(), delays.at( 0 ), resources ) )
				return closeEHandleWithError( handle );

			int delta = 0;

			for( qsizetype i = 1; i < pngFileNames.size(); ++i )
			{
				bool result = false;

				std::tie( result, delta ) = addFrame( handle, key,
					loadImage( pngFileNames.at( i ) ), delays.at( i ) + delta, resources );

				if( !result )
					return closeEHandleWithError( handle );
			}

			closeEHandle( handle );

			return true;
		}
	}
	else
		qDebug() << "Count of PNG files and delays are not the same, or list of files is empty.";

	return false;
}

void
Gif::clean()
{
	m_framesCount = 0;
	m_delays.clear();
	m_dir.remove();
	m_dir = QTemporaryDir( "./" );
}

} /* namespace QGifLib */
