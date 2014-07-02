#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtWebKitWidgets>
#include <QBuffer>
#include <QUrl>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);   
    qDebug() << "MainWindow::MainWindow()";

    this->m_state = 0;
    this->m_camera = 0;
    this->m_viewfinder = 0;
    this->m_imageCapture = 0;
    this->m_imageReady = false;
    this->m_frameInterval = 0;
    this->m_trayIcon = 0;
    this->m_websockServer = 0;
    this->p_ld = 0;

    /*
    Qt::WindowFlags flags = 0;
    flags |= Qt::WindowSystemMenuHint;
    flags |= Qt::Widget;
    this->setWindowFlags(flags);
    this->setParent(0); // Create TopLevel-Widget
    this->setAttribute(Qt::WA_NoSystemBackground, true);
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    this->setAttribute(Qt::WA_PaintOnScreen); // as pointed by Caveman (thanks!)
    */

    this->m_appParameters = new QtKApplicationParameters(this,QString("vejam"));
    if(this->m_appParameters->fileLoad(false))
    {       
		this->setDefaultParameters();
    }

    if(loadAvaliableCameras())
    {
        QMessageBox msgBox;
        msgBox.setText("Windows dice que no hay camaras disponibles?");
        msgBox.exec();
    }

    this->loadAppParameters();
    this->initTrayIcon();
    this->runMachine();
    this->syncMachine();

    QWebFrame* tempFrame = this->ui->webView->page()->mainFrame();
    QObject::connect(tempFrame, SIGNAL(javaScriptWindowObjectCleared()),this, SLOT(addJSObject()));
    QObject::connect(this->ui->webView, SIGNAL(loadStarted()),this, SLOT(loadStarted()));
    QObject::connect(this->ui->webView, SIGNAL(loadProgress(int)),this, SLOT(loadProgress(int)));
    QObject::connect(this->ui->webView, SIGNAL(loadFinished(bool)),this, SLOT(loadFinished(bool)));

    QString url = "http://";
    url += this->m_serverUrl;

    if(this->m_autoStart)
    {
        url += "/app-gui-run.php?username=";
        url += this->m_username;
        url += "&password=";
        url += this->m_password;
    }
    else
    {
        url += "/app-gui-welcome.html";
    }

    qDebug() << url;    
    this->ui->webView->load(QUrl(url));

    qDebug() << "MainWindow::MainWindow(END)";
}

MainWindow::~MainWindow()
{
    this->releaseAllocatedPointers();
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    case QEvent::WindowStateChange:
    {
        if(isMinimized())
        {
            this->hide();
        }
        else
        {
            this->setWindowState((this->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
        }
    }
        break;
    default:
        break;
    }
}

void MainWindow::runMachineSet(int newState)
{
   this->m_state = newState;
}

void MainWindow::runMachine()
{
    static int captureTime;
    switch(this->m_state)
    {
        case stIdle: break;
        case stCapture:
             captureTime = 0;
             this->m_imageReady = false;
             if(this->m_camera->state() == QCamera::ActiveState)
             {
                if(this->m_websockServer)
                {
                     if((this->m_websockServer->getState() == QtKWebsockServer::stConnected) ||
                      (this->isHidden() == false))
                    {
                        this->m_imageCapture->capture();
                        this->m_state = stWaitCapture;
                    }
                }
             }
             break;

        case stWaitCapture:
             if(this->m_imageReady)
             {
                 this->m_state = stBaseEncode;
                 this->m_imageReady = false;
             }
             else break;

        case stBaseEncode:
             this->image2Base64();
             this->m_state = stSend;

        case stSend:
             if(!this->isHidden())
             {
                emit this->webImageReady();             
             }

             if(this->m_websockServer)
             {
                 if(this->m_websockServer->getState() == QtKWebsockServer::stConnected)
                 {
                     const char* replyStr = "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\nContent-Length: %d\r\nConnection:close\r\n\r\n";
                     QByteArray img;
                     QString reply;
                     this->m_websockServer->dataSend(this->getUrlDataImage().toUtf8());
                     this->m_state = stWaitAck;
/*
                    switch(this->m_websockServer->getSocketMode())
                    {
                        case QtKWebsockServer::modeWebSocket:
                            this->m_websockServer->dataSend(this->getUrlDataImage().toUtf8());
                            this->m_state = stWaitAck;
                            break;

                        case QtKWebsockServer::modeAjaxRequest:
                            //Si llegamos  aqui es que el navegador esta esperando una imagen.
                            //Se tiene que sincronizar con el ciclo de capturas de imagenes.

                            img = this->image2ByteArray();
                            reply.sprintf(replyStr,img.size());
                            this->m_websockServer->dataSendAjax(reply.toUtf8());
                            this->m_websockServer->dataSendAjax(img);
                            this->m_state = stWaitState;
                            break;

                        default: break;
                     }
*/
                 }
                 else
                 {
                    this->m_state = stWaitState;
                 }
             }            
             break;


        case stWaitAck:
             //Es controla desde OnDataReceived();
             //this->m_state = stWaitState;
             break;

        case stWaitState:
            if(captureTime < this->m_frameInterval)
            {
                break;
            }
            else
            {
                this->m_state = stCapture;
            }
            break;

        default: break;
    }

    QTimer::singleShot(APP_RUN_TIMER_PRESCALER, this, SLOT(runMachine()));
    captureTime += 1;
    QThread::msleep(1);
}

void MainWindow::syncMachineSet(int newState)
{
   this->m_registerState = newState;
}

void MainWindow::syncMachine()
{
    static int registerTime;
    switch(this->m_registerState)
    {
        case stIdle:
        case stSendRegister:
        case stWaitAck:
        case stGoRegister:
            if(registerTime < this->m_registerInterval)
            {
                break;
            }
            else
            {
                this->m_state = stSendRegister;
            }
            break;
        default: break;
    }

    QTimer::singleShot(APP_RUN_REGISTER_PRESCALER, this, SLOT(syncMachine()));
    registerTime += 1;
    QThread::msleep(APP_RUN_TIMER_PRESCALER);
}


void MainWindow::loadStarted()
{
    ui->progressBar->setMaximum(100);
    ui->progressBar->setMinimum(0);
    ui->progressBar->setValue(0);
    this->p_ld = new loadingDialog(this);
    this->p_ld->show();
}

void MainWindow::loadProgress(int progress)
{
    ui->progressBar->setValue(progress);
    if(this->p_ld)
    {
        this->p_ld->setProgress(progress);
    }
}

void MainWindow::loadFinished(bool result)
{
    this->p_ld->hide();
    if(this->p_ld)
    {
        delete this->p_ld;
        this->p_ld = 0;
    }
}

void MainWindow::addJSObject()
{
    this->ui->webView->page()->mainFrame()->addToJavaScriptWindowObject(QString("vejamApp"), this);
}

void MainWindow::setDefaultParameters()
{
    this->saveParam(QString("aplicacion"),QString("sync-interval"),QString("3600"));
    this->saveParam(QString("aplicacion"),QString("webkit-debug"),QString("1"));
    this->saveParam(QString("aplicacion"),QString("server-url"),QString("www.vejam.info/app-gui")); //http://www.vejam.info/app-gui/app-gui-welcome.html
    this->saveParam(QString("conexion"),QString("tcpPort"),QString("12345"));
    this->saveParam(QString("video"),QString("resolucion-x"),QString("320"));
    this->saveParam(QString("video"),QString("resolucion-y"),QString("240"));
    this->saveParam(QString("video"),QString("calidad"),QString("0"));
    this->saveParam(QString("video"),QString("framerate-max"),QString("12"));
	this->saveParam(QString("device"),QString("selected"),QString("1"));	//Indica la camara per defecte.	
    this->fileSave();
}

bool MainWindow::loadAvaliableCameras()
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
  }

  if(this->m_devices.size())
  {
      this->saveParam(QString("device"),QString("qtty"),QString("%1").arg(order));
      this->fileSave();
      return 0;
  }

  return 1;
}

#include <QImageEncoderControl>
void  MainWindow::setCamera(const QByteArray &cameraDevice)
{
    this->releaseAllocatedPointers();
    if (cameraDevice.isEmpty())
        this->m_camera = new QCamera;
    else
        this->m_camera = new QCamera(cameraDevice);

    connect(this->m_camera, SIGNAL(stateChanged(QCamera::State)), this, SLOT(updateCameraState(QCamera::State)));
    connect(this->m_camera, SIGNAL(error(QCamera::Error)), this, SLOT(displayCameraError(QCamera::Error)));
  
	
	this->m_encodeSettings.setCodec("image/jpeg");
    this->m_encodeSettings.setResolution(this->loadParam(QString("video"),QString("resolucion-x")).toInt(),
                                         this->loadParam(QString("video"),QString("resolucion-y")).toInt());

    this->m_encodeSettings.setQuality((QMultimedia::EncodingQuality)this->loadParam(QString("video"),QString("calidad")).toInt());

	this->m_imageCapture = new QCameraImageCapture(this->m_camera);
    this->m_imageCapture->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);    
    this->m_imageCapture->setEncodingSettings(this->m_encodeSettings);

	qDebug() << "Supported image settings:";
	qDebug() << this->m_imageCapture->supportedBufferFormats();
	qDebug() << this->m_imageCapture->supportedImageCodecs();
	qDebug() << this->m_imageCapture->supportedResolutions();
	

    //connect(m_imageCapture, SIGNAL(readyForCaptureChanged(bool)), this, SLOT(readyForCapture(bool)));
    connect(m_imageCapture, SIGNAL(imageCaptured(int,QImage)), this, SLOT(processCapturedImage(int,QImage)));
    //connect(m_imageCapture, SIGNAL(imageSaved(int,QString)), this, SLOT(imageSaved(int,QString)));
    connect(m_imageCapture, SIGNAL(error(int,QCameraImageCapture::Error,QString)), this,SLOT(displayCaptureError(int,QCameraImageCapture::Error,QString)));
    //connect(this->m_camera, SIGNAL(lockStatusChanged(QCamera::LockStatus, QCamera::LockChangeReason)), this, SLOT(updateLockStatus(QCamera::LockStatus, QCamera::LockChangeReason)));

    this->m_viewfinder = new QCameraViewfinder();
    this->m_camera->setViewfinder(this->m_viewfinder);
    this->m_camera->setCaptureMode(QCamera::CaptureVideo);
    this->m_camera->start();
}

void MainWindow::releaseAllocatedPointers()
{
    if(this->m_camera) delete this->m_camera;
    if(this->m_imageCapture) delete this->m_imageCapture;
    if(this->m_viewfinder) delete this->m_viewfinder;   
}


void MainWindow::processCapturedImage(int id, QImage image)
{
    this->m_imageReady = true;
    this->m_currentFrame = image;   
}

void  MainWindow::displayCaptureError(int id,QCameraImageCapture::Error error, QString errorString)
{
    qDebug() << "MainWindow::displayCaptureError( " << errorString << " )";
}

void MainWindow::image2Base64()
{
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QBuffer::WriteOnly);
    this->m_currentFrame.save( &buffer, "JPG", -1);
    this->m_currentBase64Frame = ba.toBase64();
    buffer.close();
}

QByteArray MainWindow::image2ByteArray()
{
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QBuffer::WriteOnly);
    this->m_currentFrame.save( &buffer, "JPG", -1);
    buffer.close();
    return ba;
}



QString MainWindow::getUrlDataImage()
{
    QString data = "data:image/jpeg;base64,";
    data += this->m_currentBase64Frame;
    return data;
}
void MainWindow::showApp(bool show)
{
    if(show) this->show();
    else this->hide();
}

void MainWindow::updateCameraState(QCamera::State state)
{
    qDebug() << "MainWindow::updateCameraState( " << state << " )";
}

void  MainWindow::displayCameraError(QCamera::Error error)
{
    qDebug() << "MainWindow::displayCameraError( " << error << " )";
}

void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    qDebug() << "MainWindow::trayIconActivated( " << reason << " )";
    this->show();
    this->activateWindow();
    this->raise();
}

void MainWindow::initTrayIcon()
{
    this->m_trayIcon = new QSystemTrayIcon(this);
    connect(this->m_trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));
    this->setTrayIconState(trIndeterminado);
    //this->m_icon = QIcon(":/png/img/vejam_small_h22_white.png");
    //this->m_trayIcon->setIcon(this->m_icon);
    //this->m_trayIcon->show();
}

void MainWindow::setTrayIconState(int newState)
{    
    switch(newState)
    {
        case trIdle: this->m_icon = QIcon(":/png/img/vejam_small_h22_white.png"); break;
        case trWaitingConnection: this->m_icon = QIcon(":/png/img/vejam_small_h22_yellow.png"); break;
        case trClientConnected: this->m_icon = QIcon(":/png/img/vejam_small_h22_green.png"); break;
        case trError: this->m_icon = QIcon(":/png/img/vejam_small_h22_red.png"); break;
        case trIndeterminado:
        default: this->m_icon = QIcon(":png/img/vejam_small_h22_blue.png"); break;
                break;
    };

    this->m_trayIcon->setIcon(this->m_icon);
    this->m_trayIcon->show();
}

 void MainWindow::startServer()
 {
	 this->runMachineSet(stIdle);
     if(this->m_websockServer) delete this->m_websockServer;
     this->m_websockServer = new  QtKWebsockServer(this);
     connect(this->m_websockServer, SIGNAL(socketReset()), this, SLOT(OnSocketReset()));
     connect(this->m_websockServer, SIGNAL(stateChanged(int)), this, SLOT(OnStateChanged(int)));
     connect(this->m_websockServer, SIGNAL(dataReceived(QByteArray)), this, SLOT(OnDataReceived(QByteArray)));
     connect(this->m_websockServer, SIGNAL(dataReceivedAjax(QByteArray)), this, SLOT(OnDataReceivedAjax(QByteArray)));
     
	 qint16 v;
	 QString s;
     v = this->m_appParameters->loadParam(QString("conexion"),QString("tcpPort"),0).toInt();
     this->m_websockServer->start(v);

	 v = this->m_appParameters->loadParam(QString("device"),QString("selected"),0).toInt();
	 s = this->m_appParameters->loadParam(QString("device"),QString("name"),v);
	 
	 this->setCamera(s.toUtf8());        
	 this->runMachineSet(stCapture);
 }

 void MainWindow::OnSocketReset()
 {
    this->runMachineSet(stCapture);
 }

 void MainWindow::OnStateChanged(int newState)
 {
     qDebug() << "OnStateChanged(" << newState  << ")";
     QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information;
     QString msg;
     //MessageIcon { NoIcon, Information, Warning, Critical
     switch(newState)
     {
         case QtKWebsockServer::stUnknown:
              this->setTrayIconState(MainWindow::trIndeterminado);
              icon = QSystemTrayIcon::Warning;
              msg = "Error desconocido";
              break;

         case QtKWebsockServer::stListening:
             this->setTrayIconState(MainWindow::trWaitingConnection);
             icon = QSystemTrayIcon::Information;
             msg = "Funcionando, a la espera de conexiones...";
             break;

         case QtKWebsockServer::stHandShake:
              break;

         case QtKWebsockServer::stConnected:
             this->setTrayIconState(MainWindow::trClientConnected);
             icon = QSystemTrayIcon::Information;
			 if(this->m_websockServer->getSocketMode() == QtKWebsockServer::modeWebSocket)
			 {
				msg = "Conexión remota establecida!";
			 }
             break;

         case QtKWebsockServer::stDisconnected:
             this->setTrayIconState(MainWindow::trIdle);
             icon = QSystemTrayIcon::Information;
			 if(this->m_websockServer->getSocketMode() == QtKWebsockServer::modeWebSocket)
			 {
				msg = "Conexión remota finalizada!";
			 }
             break;

         case QtKWebsockServer::stError:
         default:
             this->setTrayIconState(MainWindow::trError);
             icon = QSystemTrayIcon::Critical;
             msg = "Error!";
             break;
     }

	  
                    

	 if(msg.size()) this->m_trayIcon->showMessage(QString("VEJAM Info:"),msg,icon);
 }

void MainWindow::OnDataReceived(QByteArray data)
{
    qDebug() << "OnDataReceived(" << data << ")";
    if(data.at(0)=='#')
    {
        //qDebug() << data;
        this->runMachineSet(stWaitState);
    }
}

void MainWindow::OnDataReceivedAjax(QByteArray data)
{
    qDebug() << "OnDataReceivedAjax()";
}

void MainWindow::loadAppParameters()
{
    QString sData;
    qint16 data;  //ms
    data = this->loadParam(QString("video"),QString("framerate-max")).toInt();
    if(data)
    {
        if(data > 24) data = 24;
        this->m_frameInterval = ((1000 / data) / APP_RUN_TIMER_PRESCALER);
    }

    data = this->loadParam(QString("aplicacion"),QString("register-interval")).toInt();
    if(data)
    {
        this->m_registerInterval = data;
    }

    if(!this->loadParam(QString("aplicacion"),QString("webkit-debug")).compare("1"))
    {
        QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    }

    this->m_serverUrl = this->loadParam(QString("aplicacion"),QString("server-url"));

    if(!this->loadParam(QString("aplicacion"),QString("auto-start")).compare("1"))
    {
        this->m_autoStart = true;
        this->m_username = this->loadParam(QString("aplicacion"),QString("username"));
        this->m_password = this->loadParam(QString("aplicacion"),QString("password"));
    }
    else this->m_autoStart = false;

}


void MainWindow::saveParam(QString groupName, QString paramName, QString paramValue, quint16 order)
{
    if(this->m_appParameters)
    {
        this->m_appParameters->saveParam(groupName, paramName,  paramValue, order);
        this->loadAppParameters();
    }
}

QString MainWindow::loadParam(QString groupName, QString paramName, quint16 order)
{
    if(this->m_appParameters)
    {
        return this->m_appParameters->loadParam(groupName, paramName, order);
    }
}

bool MainWindow::fileLoad(bool showAlerts)
{
    if(this->m_appParameters)
    {
        return this->m_appParameters->fileLoad(showAlerts);
    }
}

bool MainWindow::fileSave()
{
    if(this->m_appParameters)
    {
        return this->m_appParameters->fileSave();
    }
}

void  MainWindow::setSyncRealm(QString realm)
{
    this->m_syncRealm = realm;
}
