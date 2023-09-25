
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
#include <qgiflib.hpp>

#include <QLabel>
#include <QApplication>
#include <QTimer>


int main( int argc, char ** argv )
{
	QApplication app( argc, argv );

	QGifLib::Gif gif;
	gif.load( "happy.gif" );

	QVector< int > delays( gif.count(), gif.delay( 0 ) );

	QGifLib::Gif::write( "out.gif", gif.fileNames(), delays, 0 );

	QLabel l;
	l.resize( gif.at( 0 ).size() );
	l.setPixmap( QPixmap::fromImage( gif.at( 0 ) ) );
	l.show();

	QTimer t;
	t.setInterval( gif.delay( 0 ) );

	QObject::connect( &t, &QTimer::timeout,
		[&]()
		{
			static int i = 0;

			++i;

			if( i == gif.count() )
				i = 0;

			l.setPixmap( QPixmap::fromImage( gif.at( i ) ) );
		} );

	t.start();

	return QApplication::exec();
}
