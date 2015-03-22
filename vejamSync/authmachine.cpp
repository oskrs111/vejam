#include <QTimer.h>
#include <QCryptographicHash>
#include "authMachine.h"

authMachine::authMachine(QtKApplicationParameters *params)
{
    this->m_authState = sstIdleAuth;
    this->m_appParameters = params;
    QTimer::singleShot(APP_RUN_AUTH_PRESCALER, this, SLOT(OnAuthMachineRun()));
}

void authMachine::authMachineSetState(int newState)
{   
   qDebug() << "authMachine::authMachineSet(" << newState  << ")";
   this->m_authState = newState;
}

void authMachine::OnAuthMachineRun()
//D'aqui ha de sortir la informació encriptada així que primer di demanem al php desde quina adreça ip estem comunicant
//i sincronizem despres... Total no hi ha pressa no?
{
    static int authTime = 0;
    static QNetworkAccessManager *manager = 0;
    QString url = "";
    QString msg;

#ifndef VEJAM_NO_GUI
    QMessageBox msgBox(this);
#endif

	this->m_authInterval = 1;
	
    switch(this->m_authState)
    {
        case sstIdleAuth:
        case sstDoneAuth:
             break;

        case sstGoAuth:
             authTime = 0;
             this->authMachineSetState(sstAuthWait);
              qDebug() << "Autentificando...";
             break;

        case sstAuthWait:
            if(authTime > this->m_authInterval)
            {
                this->authMachineSetState(sstSendAuth);
            }
            break;
     
        case sstSendAuth:
            url = "http://";
            url += this->loadParam(QString("aplicacion"),QString("server-url"));
            url += "/app-gui-auth.php?username=";            
            url += this->loadParam(QString("aplicacion"),QString("username"));
            //url += "&password=";
            //url += this->loadParam(QString("aplicacion"),QString("password"));
            url += "&hash=";
            url += QString(QCryptographicHash::hash(this->loadParam(QString("aplicacion"),
                                                                    QString("password")).toUtf8(),
                                                                    QCryptographicHash::Md5).toHex());
			url += "&sourceId=";
            url += this->loadParam(QString("aplicacion"),QString("streamming-id"));
		
            manager = new QNetworkAccessManager(this);
            connect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(OnAuthAckReply(QNetworkReply*)));
            manager->get(QNetworkRequest(QUrl(url)));
            qDebug() << "authMachine::authMachine(sstAskForIp:" << url << ")";
            this->authMachineSetState(sstWaitAuthAck);
            break;
			
        case sstWaitAuthAck:
             //Esperem el callback desde syncAckReply(QNetworkReply*)...
             break;

        case sstWaitAuthAckDone:
            qDebug() << "Autentificacion correcta!";
            this->authMachineSetState(sstDoneAuth);
			emit this->authDone(this->m_currentRealm);

            if(manager)
            {
                delete manager;
                manager = 0;
            }
            break;

		case sstAuthError:
             this->authMachineSetState(sstGoAuth);
            qDebug() << "Error autentificando... reintento...";
             break;

        default: break;
    }

    QTimer::singleShot(APP_RUN_AUTH_PRESCALER, this, SLOT(OnAuthMachineRun()));
    authTime += 1;
    QThread::msleep(1);
}

void authMachine::OnAuthAckReply(QNetworkReply* reply)
{
    if (reply->error() == QNetworkReply::NoError)
    {
         QByteArray bytes = reply->readAll();  // bytes
         qDebug() << "authMachine::syncAckReply(reply:" << bytes << ")";
         
		QJsonDocument jsonResponse = QJsonDocument::fromJson(bytes);
        QJsonObject jsonObject = jsonResponse.object();
		QString result = jsonObject["result"].toString();
		if(result.compare("success"))
		{
            this->authMachineSetState(sstAuthError);
			return;
		}

        this->m_currentRealm = jsonObject["syncRealm"].toString();
        this->authMachineSetState(sstWaitAuthAckDone);
     }
     else
     {
         this->authMachineSetState(sstAuthError);
     }

     reply->deleteLater();
}

QString authMachine::loadParam(QString groupName, QString paramName)
{
    if(this->m_appParameters)
    {
        return this->m_appParameters->loadParam(groupName, paramName, 0);
    }

    return 0;
}
