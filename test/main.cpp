
/*
	SPDX-FileCopyrightText: 2023-2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: MIT
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

//	gif.write( "out.gif", gif.fileNames(), gif.delays(), 0 );

	QLabel l;
	l.setAlignment( Qt::AlignCenter );
	l.setText( "GIF is not available." );

	if( gif.count() )
	{
		l.resize( gif.at( 0 ).size() );
		l.setPixmap( QPixmap::fromImage( gif.at( 0 ) ) );
	}
	else
		l.resize( 400, 400 );

	l.show();

	QTimer t;
	t.setInterval( gif.count() ? gif.delay( 0 ) : 1000 );

	if( gif.count() )
	{
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
	}

	return QApplication::exec();
}
