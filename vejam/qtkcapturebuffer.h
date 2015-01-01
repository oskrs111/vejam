#ifndef QTKCAPTUREBUFFER_H
#define QTKCAPTUREBUFFER_H
#include <QAbstractVideoSurface>


class QtKCaptureBuffer : public QAbstractVideoSurface
//http://ull-etsii-sistemas-operativos.github.io/videovigilancia-blog/capturando-secuencias-de-video-con-qt.html
{
    Q_OBJECT
public:

	QtKCaptureBuffer(QObject *parent = 0);
	~QtKCaptureBuffer();

    QList<QVideoFrame::PixelFormat> supportedPixelFormats(
            QAbstractVideoBuffer::HandleType handleType =
            QAbstractVideoBuffer::NoHandle) const
    {
        // A travéss de este métodotodo nos preguntan que formatos de
        // vídeodeo soportamos. Como vamos a guardar los frames en
        // objetos QImage nos sirve cualquiera de los formatos
        // sorportados por dicha clase: http://kcy.me/z6pa
        QList<QVideoFrame::PixelFormat> formats;
        formats << QVideoFrame::Format_ARGB32;
        formats << QVideoFrame::Format_ARGB32_Premultiplied;
        formats << QVideoFrame::Format_RGB32;
        formats << QVideoFrame::Format_RGB24;
        formats << QVideoFrame::Format_RGB565;
        formats << QVideoFrame::Format_RGB555;
        return formats;
    }

	bool present(const QVideoFrame &frame);
	void capture();

Q_SIGNALS:
	void imageCaptured(int id, const QImage &preview);

    private:
	QImage::Format getQImageFormat(QVideoFrame::PixelFormat format);    
	QMutex mutex;
	bool doCapture;
};

#endif