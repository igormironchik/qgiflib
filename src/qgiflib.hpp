
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

#pragma once

// Qt include.
#include <QString>
#include <QImage>
#include <QTemporaryDir>

// giflib include.
#include <gif_lib.h>


namespace QGifLib {

//
// Gif
//

//! Gif file wrapper.
class Gif final
{
public:
	Gif() = default;
	~Gif() = default;

	//! Load GIF.
	bool load(
		//! Input file name.
		const QString & fileName );
	//! \return Delay interval in millseconds.
	int delay( qsizetype idx ) const;
	//! \return Count of frames.
	qsizetype count() const;
	//! \return Frame with given index (starting at 0).
	QImage at(
		//! Index of the requested frame (indexing starts with 0).
		qsizetype idx ) const;

	//! Write GIF from sequence of PNG files.
	static bool write(
		//! Output file name.
		const QString & fileName,
		//! Sequence of PNG file names.
		const QStringList & pngFileNames,
		//! Sequence of delays in milliseconds.
		const QVector< int > & delays );

	//! Clean internals.
	void clean();

private:
	bool closeHandleWithError( GifFileType * handle );
	bool closeHandle( GifFileType * handle );

private:
	qsizetype framesCount = 0;
	QTemporaryDir dir = QTemporaryDir( "./" );
	QVector< int > delays;
}; // class Gif

} /* namespace QGifLib */
