#include <QBuffer>
#include <QImageEncoderControl>
#include "qtkvideoserver.h"

QtkVideoServer::QtkVideoServer(QtKApplicationParameters *params, QObject *parent) :
    QObject(parent)
{
        this->m_appParameters = params;
		this->m_camera = 0;
}

bool QtkVideoServer::loadAvaliableCameras()
{
  qint16 order = 1;
  struct vjCameraDevice camDevice;

  foreach(const QByteArray &deviceName, QCamera::availableDevices())
  {
      camDevice.m_name = deviceName;
      camDevice.m_description =  QCamera::deviceDescription(deviceName);

      qDebug() << "Añade camara: " << deviceName;
      this->m_devices.append(camDevice);
      this->saveParam(QString("device"),QString("name"),QString(camDevice.m_name),order);
      this->saveParam(QString("device"),QString("description"),QString(camDevice.m_description),order);
	  order++;
  }

  if(this->m_devices.size())
  {
      this->saveParam(QString("device"),QString("qtty"),QString("%1").arg(this->m_devices.size()));      
      return 0;
  }
  return 1;
}

void QtkVideoServer::startServer()
{	
	this->setCamera(this->m_devices.at(0).m_name);
	this->m_camera->start();
	this->m_viewfinder->capture();
}

void QtkVideoServer::Capture()
{
	this->m_viewfinder->capture();
}

void  QtkVideoServer::setCamera(const QByteArray &cameraDevice)
{    
    if (cameraDevice.isEmpty())
        this->m_camera = new QCamera;
    else
        this->m_camera = new QCamera(cameraDevice);

    connect(this->m_camera, SIGNAL(stateChanged(QCamera::State)), this, SLOT(OnUpdateCameraState(QCamera::State)));
    connect(this->m_camera, SIGNAL(error(QCamera::Error)), this, SLOT(OnDisplayCameraError(QCamera::Error)));
  	
	//this->m_encodeSettings.setCodec("image/jpeg");
    
	//NOTA: Esto en Qt5.3 no funciona... se implementa en QImage
	//this->m_encodeSettings.setResolution(this->loadParam(QString("video"),QString("resolucion-x")).toInt(),
    //                                     this->loadParam(QString("video"),QString("resolucion-y")).toInt());

    //this->m_encodeSettings.setQuality((QMultimedia::EncodingQuality)this->loadParam(QString("video"),QString("calidad")).toInt());


	//this->m_imageCapture = new QCameraImageCapture(this->m_camera);
    //this->m_imageCapture->setCaptureDestination(QCameraImageCapture::CaptureToBuffer); //->https://qt-project.org/forums/viewthread/17204
    //this->m_imageCapture->setEncodingSettings(this->m_encodeSettings);

	//qDebug() << "Supported image settings:";
	//qDebug() << this->m_imageCapture->supportedBufferFormats();
	//qDebug() << this->m_imageCapture->supportedImageCodecs();
	//qDebug() << this->m_imageCapture->supportedResolutions();	

    //connect(m_imageCapture, SIGNAL(readyForCaptureChanged(bool)), this, SLOT(readyForCapture(bool)));
    //connect(m_imageCapture, SIGNAL(imageCaptured(int,QImage)), this, SLOT(processCapturedImage(int,QImage)));
    //connect(m_imageCapture, SIGNAL(imageSaved(int,QString)), this, SLOT(imageSaved(int,QString)));
    //connect(m_imageCapture, SIGNAL(error(int,QCameraImageCapture::Error,QString)), this,SLOT(displayCaptureError(int,QCameraImageCapture::Error,QString)));
    //connect(this->m_camera, SIGNAL(lockStatusChanged(QCamera::LockStatus, QCamera::LockChangeReason)), this, SLOT(updateLockStatus(QCamera::LockStatus, QCamera::LockChangeReason)));

    //this->m_viewfinder = new QCameraViewfinder();
	//this->m_viewfinder->show();
    this->m_viewfinder = new QtKCaptureBuffer(this);
	connect(this->m_viewfinder, SIGNAL(imageCaptured(int,QImage)), this, SLOT(OnProcessCapturedImage(int,QImage)));

    this->m_camera->setViewfinder(this->m_viewfinder);
    this->m_camera->setCaptureMode(QCamera::CaptureVideo);    
}

int QtkVideoServer::getServerState()
{
    if(this->m_camera)
    {
        return this->m_camera->state();
    }
    else return 0;
}

void QtkVideoServer::OnProcessCapturedImage(int id, QImage image)
{
    m_mutexA.lock();
	this->m_currentFrame = image;
	m_mutexA.unlock();
    emit frameUpdated();
}

QImage QtkVideoServer::currentFrame2Image()
{
    QImage lastFrame;
    m_mutexA.lock();
    lastFrame = this->m_currentFrame;
    m_mutexA.unlock();
    return lastFrame;
}

QByteArray QtkVideoServer::currentFrame2Base64Jpeg()
{
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QBuffer::WriteOnly);
    m_mutexA.lock();
    this->m_currentFrame.save( &buffer, "JPG", this->loadParam(QString("video"),QString("calidad")).toInt());    
    m_mutexA.unlock();
    buffer.close();
	return ba.toBase64();
}

QByteArray QtkVideoServer::currentFrame2ByteArrayJpeg()
{
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QBuffer::WriteOnly);    
    m_mutexA.lock();
	this->m_currentFrame.save( &buffer, "JPG", this->loadParam(QString("video"),QString("calidad")).toInt());
    m_mutexA.unlock();
    buffer.close();
    return ba;
}

void QtkVideoServer::OnUpdateCameraState(QCamera::State state)
{
    qDebug() << "QtkVideoServer::OnUpdateCameraState( " << state << " )";
}

void QtkVideoServer::OnDisplayCameraError(QCamera::Error error)
{
    qDebug() << "QtkVideoServer::OnUpdateCameraState( " << error << " )";
}

void QtkVideoServer::OnDisplayCaptureError(int id,QCameraImageCapture::Error error, QString errorString)
{
    qDebug() << "QtkVideoServer::OnDisplayCaptureError( " << errorString << " )";
}

void QtkVideoServer::saveParam(QString groupName, QString paramName, QString paramValue, quint16 order)
{
    if(this->m_appParameters)
    {
        this->m_appParameters->saveParam(groupName, paramName,  paramValue, order);        
    }
}

QString QtkVideoServer::loadParam(QString groupName, QString paramName, quint16 order)
{
    if(this->m_appParameters)
    {
        return this->m_appParameters->loadParam(groupName, paramName, order);
    }
	return 0;
}
