#ifndef QTKAPPLICATIONPARAMETERS_H
#define QTKAPPLICATIONPARAMETERS_H

#include <QObject>
#include <QVariant>
#include <QMap>

#define AP_FILE_EXTENSION ".cfg"

class QtKApplicationParameters : public QObject
{
    Q_OBJECT

private:
    QMap<QString,QVariant> m_projectParameters;
    QString m_appName;

public:
    explicit QtKApplicationParameters(QObject *parent = 0, QString appName = "parameters");
    void saveParam(QString groupName, QString paramName, QString paramValue, quint16 order = 0);
    QString loadParam(QString groupName, QString paramName, quint16 order);
    bool fileLoad(bool showAlerts);
    bool fileSave();

signals:
    void applicationParametersLoaded();
    void applicationParametersError();

public slots:

};

#endif // QTKAPPLICATIONPARAMETERS_H
