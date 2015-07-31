#include <QDebug>
#include <QTimer>
#include "qtkcapturebuffer.h"

QtKCaptureBuffer::QtKCaptureBuffer(QObject *parent):
	QAbstractVideoSurface(parent)
{
    this->m_doCapture = false;
	this->m_captureTimeout = 0;
	this->m_mirrorSetting = 0;
	this->m_widthScale = 0;
	this->m_scaleMode = 0;
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

void QtKCaptureBuffer::setMirrorSetting(int mirrorSetting)
{
	this->m_mirrorSetting = mirrorSetting;
}

void QtKCaptureBuffer::setWidthScale(int widthScale)
{
	this->m_widthScale = widthScale;
}

void QtKCaptureBuffer::setScaleMode(int scaleMode)
{
	this->m_scaleMode = scaleMode;
	if(this->m_scaleMode > 1 ) this->m_scaleMode = 1;
}

bool QtKCaptureBuffer::present(const QVideoFrame &frame)
//qtmultimedia\src\plugins\directshow\camera\dscamerasession.cpp
{
	static int cnt = 0;		
	if(!this->m_doCapture) return false;
	
	m_mutexA.lock();
	QVideoFrame tFrame = frame;	
    if(tFrame.map(QAbstractVideoBuffer::ReadOnly))
    {	
		this->m_doCapture = false;
		if(this->m_widthScale == 0) this->m_widthScale = frame.width();
		switch(this->m_mirrorSetting)
		{
			case mirrorVertical:
				this->m_lastFrame = QImage(frame.bits(), frame.width(), frame.height(), frame.bytesPerLine(), getQImageFormat(tFrame.pixelFormat())).mirrored(0, 1).scaledToWidth(this->m_widthScale, (Qt::TransformationMode)this->m_scaleMode);
				break;

			case mirrorHorizontal:
				this->m_lastFrame = QImage(frame.bits(), frame.width(), frame.height(), frame.bytesPerLine(), getQImageFormat(tFrame.pixelFormat())).mirrored(1, 0).scaledToWidth(this->m_widthScale, (Qt::TransformationMode)this->m_scaleMode);
				break;

			case mirrorAll:
				this->m_lastFrame = QImage(frame.bits(), frame.width(), frame.height(), frame.bytesPerLine(), getQImageFormat(tFrame.pixelFormat())).mirrored(1, 1).scaledToWidth(this->m_widthScale, (Qt::TransformationMode)this->m_scaleMode);
				break;

			case mirrorNone:			
			default:
			this->m_lastFrame = QImage(frame.bits(), frame.width(), frame.height(), frame.bytesPerLine(), getQImageFormat(tFrame.pixelFormat())).scaledToWidth(this->m_widthScale, (Qt::TransformationMode)this->m_scaleMode);
			break;
		}
		
        tFrame.unmap();
		m_mutexA.unlock();	
		
		emit imageCaptured(cnt++, this->m_lastFrame);				
		return true;

	}
	m_mutexA.unlock();
    return false;
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

