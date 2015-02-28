#include <QDebug>
#include <QTimer>
#include "qtkcapturebuffer.h"

QtKCaptureBuffer::QtKCaptureBuffer(QObject *parent):
	QAbstractVideoSurface(parent)
{
    this->m_doCapture = false;
	this->m_captureTimeout = 0;
}

QtKCaptureBuffer::~QtKCaptureBuffer()
{
}

void QtKCaptureBuffer::capture()
{		
	this->m_doCapture = true;
	if(this->m_captureTimeout == 0)
	{						
		QTimer::singleShot(CAPTURE_TIMER_PRESCALER, this, SLOT(OnCaptureTimer()));
		qDebug() << "QtKCaptureBuffer::OnCaptureTimer(..starting timeout...)";
	}
	this->m_captureTimeout = CAPTURE_TIMER_VALUE;
}

void QtKCaptureBuffer::OnCaptureTimer()
{
	if(this->m_captureTimeout > 1)
	{		
		this->m_captureTimeout--;
		QTimer::singleShot(CAPTURE_TIMER_PRESCALER, this, SLOT(OnCaptureTimer()));
	}
	else if(this->m_captureTimeout == 1)
	{
		this->m_captureTimeout = 0;
		this->m_doCapture = false;
		qDebug() << "QtKCaptureBuffer::OnCaptureTimer(..go to sleep...)";
	}
}

/*
bool QtKCaptureBuffer::present(const QVideoFrame &frame)
//qtmultimedia\src\plugins\directshow\camera\dscamerasession.cpp
{
	static int cnt = 0;
	QVideoFrame tFrame;		
	static QImage lastFrame; 
	if(frame.isValid() == 0) return false;
	cnt++;
	m_mutexA.lock();
	tFrame = frame;	
    if(tFrame.map(QAbstractVideoBuffer::ReadOnly) && this->m_doCapture && tFrame.isValid())
    {
		this->m_doCapture = false;
		lastFrame = QImage(tFrame.bits(), tFrame.width(), tFrame.height(), tFrame.bytesPerLine(), getQImageFormat(tFrame.pixelFormat())).mirrored(1, 0);
		if(lastFrame.save("captura.jpg"))
		{
			cnt = 0;
		}
		tFrame.unmap();        		
		//emit imageCaptured(cnt++, lastFrame);
		qDebug() << "QtKCaptureBuffer::process(" << lastFrame.byteCount() << " )";
		cnt = 0;
		m_mutexA.unlock();    
		return true;
	}
	m_mutexA.unlock();    
	return false;
}
*/

bool QtKCaptureBuffer::present(const QVideoFrame &frame)
//qtmultimedia\src\plugins\directshow\camera\dscamerasession.cpp
{
	static int cnt = 0;
	QVideoFrame tFrame = frame;
	QImage lastFrame;

	m_mutexA.lock();

    if(tFrame.map(QAbstractVideoBuffer::ReadOnly) && this->m_doCapture)
    {
		this->m_doCapture = false;
		lastFrame = QImage(frame.bits(), frame.width(), frame.height(), frame.bytesPerLine(), getQImageFormat(tFrame.pixelFormat())).mirrored(1, 0);;
        tFrame.unmap();
		emit imageCaptured(cnt++, lastFrame);
		
		//qDebug() << "QtKCaptureBuffer::process(FrameFormat is " << tFrame.pixelFormat() << " )";
	}

	m_mutexA.unlock();

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
