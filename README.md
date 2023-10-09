[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](https://opensource.org/licenses/MIT)

# `GIF` reading and writing library for `Qt`

In internals this library uses [`giflib`](https://giflib.sourceforge.net/) library.

This library is just a wrapper to simplify work with `GIFs` in `Qt`.

Interface is quite simple, look.

```cpp
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
	static bool write(
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
}; // class Gif
```