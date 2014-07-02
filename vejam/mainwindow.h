#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCamera>
#include <QMainWindow>
#include <QCameraImageCapture>
#include <QCameraViewfinder>
#include <QSystemTrayIcon>
#include <QImageEncoderSettings>
#include "loadingdialog.h"
#include "qtkwebsockserver.h"
#include "qtkapplicationparameters.h"

namespace Ui {
class MainWindow;
}

#define APP_RUN_TIMER_PRESCALER 20  //ms
#define APP_RUN_REGISTER_PRESCALER 1000 //ms

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    Q_INVOKABLE QString getUrlDataImage();
    Q_INVOKABLE void showApp(bool show);
    Q_INVOKABLE void startServer();
    Q_INVOKABLE void setSyncRealm(QString realm);
    Q_INVOKABLE void saveParam(QString groupName, QString paramName, QString paramValue, quint16 order = 0);
    Q_INVOKABLE QString loadParam(QString groupName, QString paramName, quint16 order = 0);
    Q_INVOKABLE bool fileLoad(bool showAlerts);
    Q_INVOKABLE bool fileSave();

private:
    QtKApplicationParameters* m_appParameters;
    QCamera* m_camera;
    QCameraImageCapture* m_imageCapture;
    QCameraViewfinder* m_viewfinder;
    QSystemTrayIcon* m_trayIcon;
    QtKWebsockServer* m_websockServer;    
    QIcon m_icon;
    QImageEncoderSettings m_encodeSettings;

    bool m_imageReady;
    bool m_autoStart;

    int m_state;
    int m_registerState;
    int m_frameInterval;
    int m_registerInterval;

    QString m_serverUrl;
    QString m_username;
    QString m_password;
    QString m_syncRealm;

    QImage m_currentFrame;
    QByteArray m_currentBase64Frame;
    Ui::MainWindow *ui;
    loadingDialog* p_ld;
    QList <struct vjCameraDevice> m_devices;

    bool loadAvaliableCameras();
    void runMachineSet(int newState);
    void syncMachineSet(int newState);
    void setCamera(const QByteArray &cameraDevice);
    void image2Base64();
    QByteArray image2ByteArray();
    void releaseAllocatedPointers();
    void initTrayIcon();
    void setTrayIconState(int newState);
    void changeEvent(QEvent *e);
    void setDefaultParameters();
    void loadAppParameters();

    enum runStates
    {
        stIdle = 0,
        stCapture,
        stWaitCapture,
        stBaseEncode,
        stSend,
        stWaitAck,
        stWaitState
    };

    enum registerStates
    {
        stIdleRegister = 0,
        stSendRegister,
        stWaitAckRegister,
        stGoRegister
    };


    enum trayIconStates
    {
        trIndeterminado = 0,
        trIdle,
        trWaitingConnection,
        trClientConnected,
        trError
    };

public slots:
    void runMachine();
    void syncMachine();
    void addJSObject();
    void loadStarted();
    void loadProgress(int progress);
    void loadFinished(bool result);
    void processCapturedImage(int id, QImage image);
    void displayCaptureError(int id,QCameraImageCapture::Error error, QString errorString);
    void updateCameraState(QCamera::State state);
    void displayCameraError(QCamera::Error error);
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);

    void OnSocketReset();
    void OnStateChanged(int newState);
    void OnDataReceived(QByteArray data);
    void OnDataReceivedAjax(QByteArray data);


signals:
    void webImageReady();

};

struct vjCameraDevice
{
    QByteArray m_name;
    QString m_description;
};

#endif // MAINWINDOW_H
