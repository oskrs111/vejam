#ifndef QTKVIDEOSERVER_H
#define QTKVIDEOSERVER_H
#include <QObject>
#include <QCamera>
#include <QCameraImageCapture>
#include <QCameraViewfinder>
#include <QImageEncoderSettings>
#include "qtkcapturebuffer.h"
#include "qtkapplicationparameters.h"

class QtkVideoServer : public QObject
{
    Q_OBJECT
public:
    explicit QtkVideoServer(QtKApplicationParameters* params, QObject *parent = 0);
    bool loadAvaliableCameras();
    void setCamera(const QByteArray &cameraDevice);
    void startServer();	
	void Capture();
    int getServerState();
	void osdTextWrite(QImage* img, QString osdText, int xPos, int yPos);

    QImage currentFrame2Image();
    QByteArray currentFrame2Base64Jpeg();
    QByteArray currentFrame2ByteArrayJpeg();

private:
    QImage m_currentFrame;
    QMutex m_mutexA;
    QCamera* m_camera;
    QCameraImageCapture* m_imageCapture;
    QtKCaptureBuffer*  m_viewfinder;
	//QCameraViewfinder* m_viewfinder;
    QImageEncoderSettings m_encodeSettings;
    QtKApplicationParameters* m_appParameters;
    QList <struct vjCameraDevice> m_devices;

    void saveParam(QString groupName, QString paramName, QString paramValue, quint16 order = 0);
    QString loadParam(QString groupName, QString paramName, quint16 order = 0);

signals:
    void frameUpdated();

public slots:    
    void OnProcessCapturedImage(int id, QImage image);
    void OnDisplayCaptureError(int id,QCameraImageCapture::Error error, QString errorString);
    void OnUpdateCameraState(QCamera::State state);
    void OnDisplayCameraError(QCamera::Error error);

};

struct vjCameraDevice
{
    QByteArray m_name;
    QString m_description;
};


#endif // QTKVIDEOSERVER_H
