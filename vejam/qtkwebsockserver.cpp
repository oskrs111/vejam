#include "qtkwebsockserver.h"
#include <QTimer>
#include <QThread>
#include <QEventLoop>
#include <QtCore/QtEndian>
#include <QCryptographicHash>

QtKWebsockServer::QtKWebsockServer( QObject *parent) :
    QObject(parent)
{
    this->m_serverState = 0;
    this->m_handshakeState = 0;
    this->m_lastError = 0;
    this->m_socket = 0;
	this->m_socket2delete = 0;
    this->m_lastErrorDescription = "no error";
}

QtKWebsockServer::~QtKWebsockServer()
{

}


void QtKWebsockServer::start(int port)
{
    this->m_server = new QTcpServer(this);
    this->m_server->setMaxPendingConnections(100);

    connect(this->m_server, SIGNAL(newConnection()), this, SLOT(OnNewConnection()));
    if (this->m_server->listen(QHostAddress::Any, port))
    {
        this->setServerState(stListening);
    }
    else
    {
        this->setServerState(stError);
    }
}

void QtKWebsockServer::OnNewConnection()
{
	if(this->m_socket2delete)
	{
		delete this->m_socket2delete;
		this->m_socket2delete = 0;
	}
	
	if(this->m_handshakeState == stIdle)
	{

		this->m_socket = this->m_server->nextPendingConnection();
		this->setHandShakeState(stProcessRequest);
		this->OnHandshakeMachine();
	}
	else
	{
		QTcpSocket* dummy = this->m_server->nextPendingConnection(); 	
		//dummy->close();
		//delete dummy;
		qDebug() << "OnNewConnection(Connection discard.)";
	}

	qDebug() << "OnNewConnection()";
}

void QtKWebsockServer::OnHandshakeMachine()
{
   QByteArray r;
   QString rr;
   static int w = 0;
   int waitStates = 25;
   if(this->m_socket == 0) return;

    switch(this->m_handshakeState)
    {
        case stIdle: break;
        case stProcessRequest:
            if(this->m_socket->bytesAvailable())
            {
                this->setServerState(stHandShake);
				r = this->m_socket->readAll();
                //Damos la posibilidad de utilizar AJAX por el
                //mismo puerto para aquellos navegadores que no soportan
                //websockets, como en ANDROID por ejemplo.

				 while(r.size()==0)
				{
					QEventLoop loop;
					QTimer::singleShot(10, &loop, SLOT(quit()));
					loop.exec();

					r = this->m_socket->readAll();
					if(waitStates) waitStates--;
					else
					{
					  qDebug() << "QtKWebsockServer::OnHandshakeMachine(Zero data received?)";
					  return;
					}
				}

                //this->processHttpHeader(r);
                //if(this->m_socketMode == modeWebSocket)
                //{
                    r = getSecWebSocketAccept(r);
                    if( r.size() == 0 ) break;
                    rr.sprintf(WK_HANSHAKE_REPLY,r.data());
                    qDebug() << "OnHandshakeMachine(stCheckRequest) " << rr;
                    this->m_socket->write(rr.toUtf8());
                    w = WK_HANDSHAKE_WAIT_CYCLES;
                    this->setHandShakeState(stWaitResult);
					QObject::connect(this->m_socket, SIGNAL(disconnected()),this, SLOT(OnDisconnected()));
                //}
                //else if(this->m_socketMode == modeAjaxRequest)
                //Tenemos que retransmitir la recepción de datos ya que
                //se trata de una transacción única AJAX-Pino.
                //{
                //     this->setServerState(stConnected);
                //	 QObject::connect(this->m_socket, SIGNAL(disconnected()),this, SLOT(OnDisconnected()));
                //     emit this->dataReceivedAjax(this->getHTTPData(r));
                //}
            }
            break;

        case stWaitResult:
            while(w--)
            {
                break;
            }            
            QObject::connect(this->m_socket, SIGNAL(readyRead()),this, SLOT(OnReadyRead()));
            QObject::connect(this->m_socket, SIGNAL(disconnected()),this, SLOT(OnDisconnected()));
            this->setServerState(stConnected);
			this->setHandShakeState(stDone);
            break;

        case stDone:
            emit this->socketReset();
			this->setHandShakeState(stIdle); 
            return; //Molt Important per WEBSOCKET!!!
        default: break;
    }

    QTimer::singleShot(25, this, SLOT(OnHandshakeMachine()));
}

void QtKWebsockServer::setServerState(int newState)
{
    if(this->m_serverState != newState)
    {
        this->m_serverState = newState;
        emit this->stateChanged(this->m_serverState);
		qDebug() << "setServerState(" << newState << ")";
    }
}

void QtKWebsockServer::setHandShakeState(int newState)
{
    if(this->m_handshakeState != newState)
    {
        this->m_handshakeState = newState;        
		qDebug() << "setHandShakeState(" << newState << ")";
    }
}


int QtKWebsockServer::getState()
{
  return this->m_serverState;
}

int QtKWebsockServer::getLastError()
{
    return this->m_lastError;
}

QString QtKWebsockServer::getLastErrorDescription()
{
    return this->m_lastErrorDescription;
}

void QtKWebsockServer::setLastError(int error, QString description)
{
    this->m_lastError = error;
    this->m_lastErrorDescription = description;
    qDebug() << "setLastError(" << error << description << ")";
}

void QtKWebsockServer::OnReadyRead()
{
    QByteArray r;
    QByteArray f;
    int waitStates = 10;

    if(this->m_socket == 0) return;

    r = this->m_socket->readAll();
    while(r.size()==0)
    {
        QEventLoop loop;
        QTimer::singleShot(10, &loop, SLOT(quit()));
        loop.exec();

        r = this->m_socket->readAll();
        if(waitStates) waitStates--;
        else
        {
          qDebug() << "QtKWebsockServer::OnReadyRead(Zero data received?)";
          return;
        }
    }

    qDebug() << "QtKWebsockServer::OnReadyRead("<< r <<")";

            struct webSocketFrameBase* inFrame;
            char* d;
            inFrame =(struct webSocketFrameBase*)r.data();
            d = getUnmaskedWebSocketPayload(inFrame);
            if(d)
            {               
                f.append(d);
                emit this->dataReceived(f);
            }
}

/*
void QtKWebsockServer::processHttpHeader(QByteArray dataReceived)
{
    switch(dataReceived.at(5))
    {
        case ' ':
            this->m_socketMode = modeWebSocket;
            return;
            break;

        default:
            this->m_socketMode = modeAjaxRequest;
            break;
    }

    QByteArray httpHeaders = dataReceived.mid(dataReceived.indexOf("\r\n\r\n"));
    // Discard the first line
    this->m_lastHttpHeaders.clear();
    this->m_lastHttpRequest.clear();
    this->m_lastHttpRequest = dataReceived.left(dataReceived.indexOf('\n'));
    httpHeaders = dataReceived.mid(httpHeaders.indexOf('\n') + 1).trimmed();
    foreach(QByteArray line, httpHeaders.split('\n'))
    //Cortesia de:
    //http://stackoverflow.com/questions/10893525/how-can-we-parse-http-response-header-fields-using-qt-c
    {
        int colon = line.indexOf(':');
        QByteArray headerValue = line.mid(colon + 1).trimmed();
        QByteArray headerName = line.left(colon).trimmed();
        this->m_lastHttpHeaders.insertMulti(headerName, headerValue);
    }
}
*/

QByteArray QtKWebsockServer::getHTTPData(QByteArray dataReceived)
{
    return dataReceived.right(dataReceived.indexOf("\r\n\r\n"));
}

void QtKWebsockServer::OnDisconnected()
{
    this->setServerState(stDisconnected);
	this->m_handshakeState = 0;
    this->m_socket2delete = this->m_socket;
    this->m_socket = 0;
}

void QtKWebsockServer::dataSend(QByteArray data)
{
    QByteArray d;

    if(this->m_socket == 0) return;

    if(this->m_socket->isOpen())
    {
        //struct webSocketFrameReply frame;
        //memset(&frame, 0x00, sizeof(struct webSocketFrameReply));
        //frame.Opcode = 1;
        //frame.PayloadL = 0x7F;
        //frame.PayloadL2 = data.length();
        //data.prepend((char*)&frame);
        //d = (quint8*)data.data();
        d = this->getFrameHeader(1,data.length(),0,true);
        this->m_socket->write(d);
        this->m_socket->write(data);
        this->m_socket->flush();
        qDebug() << "dataSend()";

    }
    else
    {
        this->setLastError(errSocketWriteError, QString("dataSend(m_socket not open?)"));
    }
}

void QtKWebsockServer::dataSendAjax(QByteArray data)
{
    QByteArray d;
    if(this->m_socket == 0) return;

    if(this->m_socket->isOpen())
    {
        this->m_socket->write(data);
        this->m_socket->flush();
//		while(this->m_socket->bytesToWrite())
//		{
//			 QThread::msleep(1);			
//		}

        qDebug() << "dataSendAjax()";
    }
    else
    {
        this->setLastError(errSocketWriteError, QString("dataSend(m_socket not open?)"));
    }
}

void QtKWebsockServer::forceConnectionClose()
{
    if(this->m_socket == 0) return;
    if(this->m_socket->isOpen())
    {
		this->m_socket->close();
    }
}

void QtKWebsockServer::OnError(QAbstractSocket::SocketError error)
{
    QString estr;

    switch (error)
    {
    case QAbstractSocket::AddressInUseError:
        estr = "OnError(SOCKET ERROR: Address is already in use)";
        break;
    case QAbstractSocket::ConnectionRefusedError:
        estr = "OnError(SOCKET ERROR: Connection refused)";
        break;
    case QAbstractSocket::HostNotFoundError:
        estr = "OnError(SOCKET ERROR: Host not found)";
        break;
    case QAbstractSocket::RemoteHostClosedError:
        estr = "OnError(SOCKET ERROR: Remote host closed)";
        break;
    }

    this->setServerState(stError);
    this->setLastError(error, estr);
}

QByteArray QtKWebsockServer::getSecWebSocketAccept(QByteArray httpHeaderBuffer)
{
        QByteArray wtKey;
        QByteArray hash;
        int t;

        //EXAMPLE:
        //A					->"x3JJHMbDL1EzLkh9GBhXDw==258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
        //SHA( A )			-> 1d29ab734b0c9585240069a6e4e3e91b61da1969
        //BASE64( SHA( A )) ->"HSmrc0sMlYUkAGmm5OPpG2HaGWk="
        qDebug() << "GetSecWebSocketAccept():";
        qDebug() << httpHeaderBuffer;

        //Busquem el valor del header-field "Sec-WebSocket-Key: "
            if( httpHeaderBuffer.contains(WK_HANSHAKE_KEY_E))
            {
                t = httpHeaderBuffer.indexOf(WK_HANSHAKE_KEY_E);
                t += strlen( WK_HANSHAKE_KEY_E );
                while( httpHeaderBuffer.at(t) != '\r' )
                {
                    wtKey.append(httpHeaderBuffer[t++]);
                }

                qDebug() << "wtKey= " << wtKey;
                wtKey.append(WK_HANDSHAKE_MAGIC);
                qDebug() << "wtKey= " << wtKey;

                QCryptographicHash sha1(QCryptographicHash::Sha1);
                sha1.addData(wtKey);
                hash = sha1.result();
                qDebug() << "hash= " << hash;

                return hash.toBase64();

                /*
                char hexstring[64];

                char* base64out;
                size_t base64outL;

                memset( hash, 0x00, sizeof( hash ));
                memset( hexstring, 0x00, sizeof( hexstring ));

                memcpy( &wtKey[i], WK_HANDSHAKE_MAGIC, strlen( WK_HANDSHAKE_MAGIC ));

                printf("\r\n\t\t'%s'\r\n", wtKey );

                sha1::calc( wtKey ,strlen( wtKey ), hash ); // 10 is the length of the string
                printf("sha1=\t\t'");
                for( int r = 0; r < 20; r++ )
                {
                    printf("%X", hash[r] );
                }
                printf("'\r\n");


                base64out = base64_encode( hash, sizeof( hash ), &base64outL );
                base64out[base64outL] = 0;

                printf("base64=\t\t'%s'\r\n", base64out );

                return base64out;
                */
        }

    return hash;

}

char* QtKWebsockServer::getUnmaskedWebSocketPayload(struct webSocketFrameBase* inFrame)
{
    if( inFrame )
    {
        int j;
        for( int t = 0; t < inFrame->PayloadL; t++ )
        {
            j = t % 4;
            inFrame->Payload[t] = inFrame->Payload[t] ^ inFrame->MaskingKey[j];
        }

        return (char*)&inFrame->Payload;

    }
    return 0;
}

QByteArray QtKWebsockServer::getFrameHeader(int opCode, quint64 payloadLength, quint32 maskingKey, bool lastFrame)
{
    QByteArray header;
    quint8 byte = 0x00;
    bool ok = payloadLength <= 0x7FFFFFFFFFFFFFFFULL;

    if (ok)
    {
        //FIN, RSV1-3, opcode
        byte = static_cast<quint8>((opCode & 0x0F) | (lastFrame ? 0x80 : 0x00));	//FIN, opcode
        //RSV-1, RSV-2 and RSV-3 are zero
        header.append(static_cast<char>(byte));

        //Now write the masking bit and the payload length byte
        byte = 0x00;
        if (maskingKey != 0)
        {
            byte |= 0x80;
        }
        if (payloadLength <= 125)
        {
            byte |= static_cast<quint8>(payloadLength);
            header.append(static_cast<char>(byte));
        }
        else if (payloadLength <= 0xFFFFU)
        {
            byte |= 126;
            header.append(static_cast<char>(byte));
            quint16 swapped = qToBigEndian<quint16>(static_cast<quint16>(payloadLength));
            header.append(static_cast<const char *>(static_cast<const void *>(&swapped)), 2);
        }
        else if (payloadLength <= 0x7FFFFFFFFFFFFFFFULL)
        {
            byte |= 127;
            header.append(static_cast<char>(byte));
            quint64 swapped = qToBigEndian<quint64>(payloadLength);
            header.append(static_cast<const char *>(static_cast<const void *>(&swapped)), 8);
        }

        //Write mask
        if (maskingKey != 0)
        {
            //TODO: to big endian?
            const quint32 mask = qToBigEndian<quint32>(maskingKey);
            header.append(static_cast<const char *>(static_cast<const void *>(&mask)), sizeof(quint32));
        }
    }
    else
    {
        //setErrorString("WebSocket::getHeader: payload too big!");
        //Q_EMIT q_ptr->error(QAbstractSocket::DatagramTooLargeError);
        qDebug() << "WebSocket::getHeader: payload too big!";
    }

    return header;
}
