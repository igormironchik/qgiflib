
/*
	SPDX-FileCopyrightText: 2023-2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: MIT
*/

#pragma once

// Qt include.
#include <QString>
#include <QImage>
#include <QTemporaryDir>
#include <QObject>

// giflib include.
#include <gif_lib.h>


namespace QGifLib {

//
// Gif
//

//! Gif file wrapper.
class Gif final
	:	public QObject
{
	Q_OBJECT
	
signals:
	//! Write GIF progress.
	void writeProgress( int percent );
	
public:
	explicit Gif( QObject * parent = nullptr );
	~Gif() = default;

	//! Load GIF.
	bool load(
		//! Input file name.
		const QString & fileName );
	//! \return Delay interval in millseconds.
	int delay( qsizetype idx ) const;
	//! \return Delays of frames.
	const QVector< int > & delays() const;
	//! \return Count of frames.
	qsizetype count() const;
	//! \return Frame with given index (starting at 0).
	QImage at(
		//! Index of the requested frame (indexing starts with 0).
		qsizetype idx ) const;

	//! \return File names of frames.
	QStringList fileNames() const;

	//! Write GIF from sequence of PNG files.
	bool write(
		//! Output file name.
		const QString & fileName,
		//! Sequence of PNG file names.
		const QStringList & pngFileNames,
		//! Sequence of delays in milliseconds.
		const QVector< int > & delays,
		//! Animation loop count, 0 means infinite.
		unsigned int loopCount );

	//! Clean internals.
	void clean();

private:
	bool closeHandleWithError( GifFileType * handle );
	bool closeHandle( GifFileType * handle );

	static bool closeEHandleWithError( GifFileType * handle );
	static bool closeEHandle( GifFileType * handle );

private:
	qsizetype m_framesCount = 0;
	QTemporaryDir m_dir = QTemporaryDir( "./" );
	QVector< int > m_delays;
}; // class Gif

} /* namespace QGifLib */
