#include <QJsonObject>
#include "mainwindow.h"
void MainWindow::syncMachineSet(int newState)
{   
   qDebug() << "MainWindow::syncMachineSet(" << newState  << ")";
   this->m_syncState = newState;
}

void MainWindow::syncMachine()
//D'aqui ha de sortir la informació encriptada així que primer di demanem al php desde quina adreça ip estem comunicant
//i sincronizem despres... Total no hi ha pressa no?
{
    static int syncTime = 0;
    static QNetworkAccessManager *manager = 0;
    QString url = "";

    switch(this->m_syncState)
    {
        case sstIdleSync: break;

        case sstGoSync:
             syncTime = 0;
             this->syncMachineSet(sstSyncWait);
             break;

        case sstSyncWait:
            if(syncTime > this->m_syncInterval)
            {
                this->syncMachineSet(sstAskForIp);
            }
            break;

        case sstAskForIp:
            url = "http://";
            url += this->m_serverUrl;
            url += "/app-user-sync.php?vejamAskIp=1";
            manager = new QNetworkAccessManager(this);
            connect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(askForIpReply(QNetworkReply*)));
            manager->get(QNetworkRequest(QUrl(url)));
            this->syncMachineSet(sstWaitForIp);
            qDebug() << "MainWindow::syncMachine(sstAskForIp:" << url << ")";
            break;

        case sstWaitForIp:
            //Esperem el callback desde askForIpReply(QNetworkReply*)...
            break;

        case sstWaitForIpDone:
            this->syncMachineSet(sstSendSync);
            if(manager)
            {
                delete manager;
                manager = 0;
            }
            break;

        case sstSendSync:
            url = "http://";
            url += this->m_serverUrl;
            url += "/app-user-sync.php?vejamSync=1";
            url += "&userName=";
            url += this->loadParam(QString("aplicacion"),QString("username"));            
			url += "&sourceId=";
            url += this->loadParam(QString("aplicacion"),QString("streamming-id"));
            url += "&syncData=";
            url += this->getSyncString();   
			url += "&syncEncript=0";				
            manager = new QNetworkAccessManager(this);
            connect(manager, SIGNAL(finished(QNetworkReply*)),this, SLOT(syncAckReply(QNetworkReply*)));
            manager->get(QNetworkRequest(QUrl(url)));
            qDebug() << "MainWindow::syncMachine(sstAskForIp:" << url << ")";
            this->syncMachineSet(sstWaitSyncAck);
            break;

        case sstWaitSyncAck:
             //Esperem el callback desde syncAckReply(QNetworkReply*)...
             break;

        case sstWaitSyncAckDone:
            this->syncMachineSet(sstGoSync);
            if(manager)
            {
                delete manager;
                manager = 0;
            }
            break;

        case sstSyncError:
             this->syncMachineSet(sstGoSync);
             break;

        default: break;
    }

    QTimer::singleShot(APP_RUN_SYNC_PRESCALER, this, SLOT(syncMachine()));
    syncTime += 1;
    QThread::msleep(1);
}

QString MainWindow::getSyncString()
//JSON Base64 encoded string.
{    
    QJsonObject json;

    json.insert("server-ip",QJsonValue(this->m_lastIpReply));
    json.insert("webkit-port",QJsonValue(this->loadParam(QString("conexion"),QString("webkit-port"))));
    json.insert("mjpeg-port",QJsonValue(this->loadParam(QString("conexion"),QString("mjpeg-port"))));
    json.insert("streamming-mode",QJsonValue(this->loadParam(QString("aplicacion"),QString("streamming-mode"))));

    QJsonDocument jsonDoc(json);
    QString syncStr(jsonDoc.toJson().toBase64());

    return syncStr;
}

QByteArray MainWindow::getEncryptedString(QString cleanText, QString password)
//http://www.essentialunix.org/index.php?option=com_content&view=article&id=48:qcatutorial&catid=34:qttutorials&Itemid=53
//https://github.com/JPNaude/dev_notes/wiki/Using-the-Qt-Cryptographic-Architecture-with-Qt5
{
    //initialize QCA
    QCA::Initializer init = QCA::Initializer();
    //generate a random symmetric 16-bytes key
    QCA::SymmetricKey key = QCA::SymmetricKey(16);
    //generate a random 16-bytes initialization vector
    QCA::InitializationVector iv = QCA::InitializationVector(16);
    //initialize the cipher for aes128 algorithm, using CBC mode,
    //with padding enabled (by default), in encoding mode,
    //using the given key and initialization vector
    QCA::Cipher cipher = QCA::Cipher(QString("aes128"), QCA::Cipher::CBC,
                                     QCA::Cipher::DefaultPadding, QCA::Encode,
                                     key, iv);
    //check if aes128 is available
    if (!QCA::isSupported("aes128-cbc-pkcs7"))
    {
        qDebug() << "AES128 CBC PKCS7 not supported - "
                    "please check if qca-ossl plugin"
                    "installed correctly !";
        return;
    }
    //the string we want to encrypt
    QString s = "Hello, world !";
    //we use SecureArray: read more here:
    //QCA secure array details
    QCA::SecureArray secureData = s.toAscii();
    //we encrypt the data
    QCA::SecureArray encryptedData = cipher.process(secureData);
    //check if encryption succeded
    if (!cipher.ok())
    {
        qDebug() << "Encryption failed !";
        return;
    }
    //display the result
    qDebug() << QString(qPrintable(QCA::arrayToHex(encryptedData.toByteArray())));
    //set the cipher mode to encryption
    cipher.setup(QCA::Decode, key, iv);
    //decrypt the encrypted data
    QCA::SecureArray decryptedData = cipher.process(encryptedData);
    //check if decryption succeded
    if (!cipher.ok())
    {
        qDebug() << "Decryption failed !";
        return "";
    }
    //display the decrypted data (it should be "Hello, world !")
    qDebug() << QString(decryptedData.data());
}

void MainWindow::askForIpReply(QNetworkReply* reply)
{
     if (reply->error() == QNetworkReply::NoError)
     {
          QByteArray bytes = reply->readAll();  // bytes
          QJsonDocument jsonReply;
		  QJsonParseError jsonError;

          jsonReply = QJsonDocument::fromJson(bytes,&jsonError);
          this->m_lastIpReply = jsonReply.object().value("remoteIp").toString();
          this->syncMachineSet(sstWaitForIpDone);
      }
      else
      {
          this->syncMachineSet(sstSyncError);
      }

      reply->deleteLater();
}

void MainWindow::syncAckReply(QNetworkReply* reply)
{
    if (reply->error() == QNetworkReply::NoError)
    {
         QByteArray bytes = reply->readAll();  // bytes
		 qDebug() << "MainWindow::syncAckReply(reply:" << bytes << ")";
         //QJsonDocument jsonReply;
         //jsonReply.fromBinaryData(bytes,QJsonDocument::BypassValidation);
         //this->m_lastIpReply = jsonReply.object().value("remoteIp").toString();
         this->syncMachineSet(sstWaitSyncAckDone);
     }
     else
     {
         this->syncMachineSet(sstSyncError);
     }

     reply->deleteLater();
}

