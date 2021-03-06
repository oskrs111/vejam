/*
NOTICE: Although this piece of code has been written by me (Oscar Sanz LLopis 2014)
is highly based on official WebSockets implementation so i want to maintain original
License issues:
*/
/*
QWebSockets implements the WebSocket protocol as defined in RFC 6455.
Copyright (C) 2013 Kurt Pattyn (pattyn.kurt@gmail.com)

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
#ifndef QTKWEBSOCKSERVER_H
#define QTKWEBSOCKSERVER_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>

#define  WK_HANDSHAKE_MAGIC "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
#define  WK_HANSHAKE_KEY_E	"Sec-WebSocket-Key: "
#define  WK_HANSHAKE_REPLY	"HTTP/1.1 101 Switching Protocols\r\nConnection: Upgrade\r\nUpgrade: websocket\r\nSec-WebSocket-Accept: %s\r\nServer: QtKWebsockServer 1.0\r\n\r\n"

#define WK_HANDSHAKE_WAIT_CYCLES 4
/*
 0                   1                   2                   3
      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
     +-+-+-+-+-------+-+-------------+-------------------------------+
     |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
     |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
     |N|V|V|V|       |S|             |   (if payload len==126/127)   |
     | |1|2|3|       |K|             |                               |
     +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
     |     Extended payload length continued, if payload len == 127  |
     + - - - - - - - - - - - - - - - +-------------------------------+
     |                               |Masking-key, if MASK set to 1  |
     +-------------------------------+-------------------------------+
     | Masking-key (continued)       |          Payload Data         |
     +-------------------------------- - - - - - - - - - - - - - - - +
     :                     Payload Data continued ...                :
     + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
     |                     Payload Data continued ...                |
     +---------------------------------------------------------------+
*/


struct webSocketFrameBase
//http://tools.ietf.org/html/draft-ietf-hybi-thewebsocketprotocol-17#section-5.3
{

    quint8   Opcode:	4,
             FIN:		1,
             RSV1:		1,
             RSV2:		1,
             RSV3:		1;


    quint8   PayloadL:	7,
             Mask:		1;

    quint8   MaskingKey[4];
    quint8   Payload[1500];
};

struct webSocketFrameReply
//http://tools.ietf.org/html/draft-ietf-hybi-thewebsocketprotocol-17#section-5.3
{

    quint8   Opcode:	4,
             FIN:		1,
             RSV1:		1,
             RSV2:		1,
             RSV3:		1;


    quint8   PayloadL:	7,  //Posar a 127
             Mask:		1;  //Posar a zero

    quint32 PayloadL2;
    quint32 PayloadL3;

};

class QtKWebsockServer : public QObject
{
    Q_OBJECT
public:
    explicit QtKWebsockServer(QObject *parent = 0);
    ~QtKWebsockServer();
    int getServerState();
    void setServerState(int newState);
	void setHandShakeState(int newState);
    int getLastError();
    QString getLastErrorDescription();
    void setLastError(int error, QString description);
    void start(int port);
    int getState();
    void forceConnectionClose();

    void dataSend(QByteArray data);
    void dataSendAjax(QByteArray data);    

    enum serverStates
    {
        stUnknown = 0,
        stListening,
        stHandShake,
        stConnected,
        stDisconnected,
        stError
    };

private:
    int m_serverState;
    int m_handshakeState;
    int m_lastError;   
    QString m_lastErrorDescription;
    QTcpServer* m_server;
    QTcpSocket* m_socket;
	QTcpSocket* m_socket2delete;
    QByteArray m_lastHttpRequest;
    QMap<QByteArray, QByteArray> m_lastHttpHeaders;
    
    QByteArray getHTTPData(QByteArray dataReceived);
    char* getUnmaskedWebSocketPayload(struct webSocketFrameBase* inFrame);
    QByteArray getSecWebSocketAccept(QByteArray httpHeaderBuffer);
    QByteArray getFrameHeader(int opCode, quint64 payloadLength, quint32 maskingKey, bool lastFrame);



enum handshakeStates
{
    stIdle = 0,
    stProcessRequest,
    stWaitResult,
    stDone
};

enum errorIds
{
    errSocketWriteError = 1000
};

signals:
    void socketReset();
    void stateChanged(int newState);
    void dataReceived(QByteArray data);
    void dataReceivedAjax(QByteArray data);

public slots:
    void OnNewConnection();
    void OnHandshakeMachine();
    void OnReadyRead();
    void OnDisconnected();
    void OnError(QAbstractSocket::SocketError error);

};

#endif // QTKWEBSOCKSERVER_H
