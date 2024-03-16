
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


namespace QGifLib {

namespace /* anonymous */ {

struct Color {
	unsigned char red = 0;
	unsigned char green = 0;
	unsigned char blue = 0;
	
	bool operator == ( const Color & c ) const
	{
		return ( red == c.red && green == c.green && blue == c.blue );
	}
	
	bool operator < ( const Color & c ) const
	{
		return ( red < c.red || ( red == c.red && green < c.green ||
			( red == c.red && green == c.green && blue < c.blue ) ) );
	}
};

size_t qHash( const Color & c )
{
	return ( c.red << 16 | c.green << 8 | c.blue );
}

enum ColorComponent {
	Red,
	Green,
	Blue
};

struct ColorRange {
	unsigned char lowest = 0;
	unsigned char highest = 0;
};

QPair< ColorComponent, ColorRange > longestSide( const QMap< Color, long long int > & s )
{
	ColorRange red = { s.firstKey().red, 0 };
	ColorRange green = { s.firstKey().green, 0 };
	ColorRange blue = { s.firstKey().blue, 0 };
	
	for( const auto & k : s.keys() )
	{
		if( k.red < red.lowest )
			red.lowest = k.red;
		
		if( k.red > red.highest )
			red.highest = k.red;
		
		if( k.green < green.lowest )
			green.lowest = k.green;
		
		if( k.green > green.highest )
			green.highest = k.green;
		
		if( k.blue < blue.lowest )
			blue.lowest = k.blue;
		
		if( k.blue > blue.highest )
			blue.highest = k.blue;
	}
	
	unsigned char redDistance = qRound( (float) ( red.highest - red.lowest ) * 0.299f );
	unsigned char greenDistance = qRound( (float) ( green.highest - green.lowest ) * 0.587f );
	unsigned char blueDistance = qRound( (float) ( blue.highest - blue.lowest ) * 0.114f );
	
	const QMap< unsigned char, ColorComponent > m = { { redDistance, Red }, { greenDistance, Green },
		{ blueDistance, Blue } };
	
	switch( m.last() )
	{
		case Red :
			return { Red, red };
			
		case Green :
			return { Green, green };
			
		case Blue :
			return { Blue, blue };
			
		default :
			return { Red, red };
	}
}

void splitByLongestSide( const QMap< Color, long long int > & s,
	QVector< QMap< Color, long long int > > & appendTo )
{
	QMap< Color, long long int > left, right;
	
	if( !s.isEmpty() )
	{
		const auto side = longestSide( s );
		const unsigned char middle = ( side.second.highest - side.second.lowest ) / 2 +
			side.second.lowest;
		
		for( auto it = s.cbegin(), last = s.cend(); it != last; ++it )
		{
			switch( side.first )
			{
				case Red :
				{
					if( it.key().red < middle )
						left.insert( it.key(), it.value() );
					else
						right.insert( it.key(), it.value() );
				}
					break;
					
				case Green :
				{
					if( it.key().green < middle )
						left.insert( it.key(), it.value() );
					else
						right.insert( it.key(), it.value() );
				}
					break;
					
				case Blue :
				{
					if( it.key().blue < middle )
						left.insert( it.key(), it.value() );
					else
						right.insert( it.key(), it.value() );
				}
					break;
					
				default :
					break;
			}
		}
	}
	
	appendTo.push_back( left );
	appendTo.push_back( right );
}

QRgb colorForSet( const QMap< Color, long long int > & s )
{
	long long int red = 0, green = 0, blue = 0;
	long long int count = 0;
	
	if( !s.isEmpty() )
	{
		for( auto it = s.cbegin(), last = s.cend(); it != last; ++it )
		{
			red += it.key().red * it.value();
			green += it.key().green * it.value();
			blue += it.key().blue * it.value();
			count += it.value();
		}
		
		return qRgb( red / count, green / count, blue / count );
	}
	else
		return qRgb( 0, 0, 0 );
}

uint indexOfColor( const QColor & c, const QVector< QMap< Color, long long int > > & indexed )
{
	uint i = 0;
	const auto cc = Color{ static_cast< unsigned char > ( c.red() ),
		static_cast< unsigned char > ( c.green() ),
		static_cast< unsigned char > ( c.blue() ) };
	
	for( ; i < indexed.size(); ++i )
	{
		if( indexed[ i ].contains( cc ) )
			return i;
	}
	
	return 0;
}

} /* namespace anonymous */

QImage quantizeImageToKColors( const QImage & img, long long int k )
{
	if( k == 0 || k == 1 )
		return QImage();
	
	long long int n = 1;
	
	while( n < k )
		n <<= 1;
	
	k = n;
	
	QVector< QMap< Color, long long int > > indexed;
	indexed.push_back( {} );
	
	for( long long int y = 0; y < img.height(); ++y )
	{
		for( long long int x = 0; x < img.width(); ++x )
		{
			const auto ic = img.pixelColor( x, y );
			const auto c = Color{ static_cast< unsigned char > ( ic.red() ),
				static_cast< unsigned char > ( ic.green() ),
				static_cast< unsigned char > ( ic.blue() ) };
			
			if( !indexed.front().contains( c ) )
				indexed.front()[ c ] = 1;
			else
				++indexed.front()[ c ];
		}
	}
	
	while( n != 1 )
	{
		QVector< QMap< Color, long long int > > tmp;
		
		for( const auto & s : std::as_const( indexed ) )
			splitByLongestSide( s, tmp );
		
		std::swap( indexed, tmp );
		
		n /= 2;
	}
	
	QList< QRgb > newColors;
	
	for( const auto & s : std::as_const( indexed ) )
		newColors.push_back( colorForSet( s ) );
	
	QImage res( img.size(), QImage::Format_Indexed8 );
	res.setColorCount( k );
	res.setColorTable( newColors );
	
	for( long long int y = 0; y < img.height(); ++y )
	{
		for( long long int x = 0; x < img.width(); ++x )
			res.setPixel( x, y, indexOfColor( img.pixelColor( x, y ), indexed ) );
	}
	
	return res;
}


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
					{
						img.convertTo( QImage::Format_ARGB32 );
						key = img;
					}
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

struct Resources {
	std::shared_ptr< ColorMapObject > cmap;
	std::shared_ptr< GifColorType > colors;
	std::shared_ptr< GifPixelType > pixels;
	int colorMapSize = 256;

	void init( const QImage & img )
	{
		const auto q = quantizeImageToKColors( img, colorMapSize );

		cmap = std::make_shared< ColorMapObject > ();
		cmap->ColorCount = colorMapSize;
		cmap->BitsPerPixel = 8;

		colors = std::shared_ptr< GifColorType > ( new GifColorType[ colorMapSize ],
			ArrayDeleter< GifColorType > () );

		cmap->Colors = colors.get();

		std::map< QRgb, unsigned char > ccm;
		
		const auto ct = q.colorTable();

		for( int c = 0; c < colorMapSize; ++c )
		{
			colors.get()[ c ].Red = qRed( ct[ c ] );
			colors.get()[ c ].Green = qGreen( ct[ c ] );
			colors.get()[ c ].Blue = qBlue( ct[ c ] );

			ccm[ ct[ c ] ] = static_cast< unsigned char > ( c );
		}

		pixels = std::shared_ptr< GifPixelType > ( new GifPixelType[ img.width() * img.height() ],
			ArrayDeleter< GifPixelType > () );

		for( int y = 0; y < img.height(); ++y )
		{
			for( int x = 0; x < img.width(); ++x )
			{
				pixels.get()[ img.width() * y + x ] = static_cast< unsigned char > (
					q.pixelIndex( x, y ) );
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
