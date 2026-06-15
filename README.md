# `GIF` reading and writing library for `Qt`

In internals this library uses [`giflib`](https://giflib.sourceforge.net/) library.
No more dependencies. All things are done in place.

This library is just a wrapper to simplify work with `GIFs` in `Qt`.

This library doesn't allocate more memory then for two complete frames.

This library implements its own quantization algorithm to reduce count of colors in frames.

Access time to frames and delays are `O(1)`. This is done by storing frames on
disk in `PNG` files.

Interface is quite simple, look.

```cpp
//! Gif file wrapper.
class Gif final : public QObject
{
    Q_OBJECT

signals:
    //! Write GIF progress.
    void writeProgress(int percent);

public:
    Gif(const QString &tmpPath = QStringLiteral("./"),
        QObject *parent = nullptr);
    ~Gif() = default;

    //! Load GIF.
    bool load(
        //! Input file name.
        const QString &fileName);
    //! \return Delay interval in millseconds. First delay with
    //! index 0 is a delay between frames with indexes 0 and 1.
    int delay(qsizetype idx) const;
    //! Set delay.
    void setDelay(qsizetype idx, int ms);
    //! \return Delays of frames.
    const QVector<int> &delays() const;
    //! \return Count of frames.
    qsizetype count() const;
    //! \return Frame with given index (starting at 0).
    QImage at(
        //! Index of the requested frame (indexing starts with 0).
        qsizetype idx) const;

    //! \return File names of frames.
    QStringList fileNames() const;

    //! Write GIF from sequence of PNG files.
    bool write(
        //! Output file name.
        const QString &fileName,
        //! Sequence of PNG file names.
        const QStringList &pngFileNames,
        //! Sequence of delays in milliseconds.
        const QVector<int> &delays,
        //! Animation loop count, 0 means infinite.
        unsigned int loopCount);

    //! Clean internals.
    void clean();
}; // class Gif
```