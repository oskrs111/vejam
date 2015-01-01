#include <QDebug>
#include "qtkcapturebuffer.h"

QtKCaptureBuffer::QtKCaptureBuffer(QObject *parent):
	QAbstractVideoSurface(parent)
{
	this->doCapture = false;
}


QtKCaptureBuffer::~QtKCaptureBuffer()
{
}

void QtKCaptureBuffer::capture()
{
	this->doCapture = true;
}

bool QtKCaptureBuffer::present(const QVideoFrame &frame)
//qtmultimedia\src\plugins\directshow\camera\dscamerasession.cpp
{
	static int cnt = 0;
	QVideoFrame tFrame = frame;
	QImage lastFrame;

	mutex.lock();

    if(tFrame.map(QAbstractVideoBuffer::ReadOnly) && this->doCapture)
    {
		this->doCapture = false;
		lastFrame = QImage(frame.bits(), frame.width(), frame.height(), frame.bytesPerLine(), getQImageFormat(tFrame.pixelFormat())).mirrored(1, 0);;
        tFrame.unmap();
		emit imageCaptured(cnt++, lastFrame);
		
		//qDebug() << "QtKCaptureBuffer::process(FrameFormat is " << tFrame.pixelFormat() << " )";
	}

	mutex.unlock();

    return true;
}

QImage::Format QtKCaptureBuffer::getQImageFormat(QVideoFrame::PixelFormat format)
{
	switch(format)
	{
		case QVideoFrame::Format_RGB24: return QImage::Format_RGB888;
	    case QVideoFrame::Format_RGB32: return QImage::Format_RGB32;
		default: break; 
	}

	return QImage::Format_RGB888;
}