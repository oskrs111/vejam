#ifndef VEJAMCRYPT_H
#define VEJAMCRYPT_H
#include <QString>
#include <QByteArray>

#define VC_HEX_DUMP_MAX 1024

class vejamCrypt
{
public:
    vejamCrypt();
    void vcSetEncryptMode(int encryptMode);
    QString vcEncryptB64(QString toEncrypt, QString password);
    QString vcDecryptB64(QString toDecrypt, QString password);
    QByteArray vcEncrypt(QString toEncrypt, QString password);
    QString vcDecrypt(QByteArray toDecrypt, QString password);

    enum vcEncryptMode
    {
        vcUndefined = 0,
        vcDES_ECB,
        vcTripleDES,
        vcAES,
        vcLast = 1000
    };

private:
    int m_Mode;
    QByteArray vcDES_Encrypt(QString toEncrypt, QString password);
    QString vcDES_Decrypt(QByteArray toDecrypt, QString password);

public:
	static void vcHexDump(unsigned char* d, char* ref, bool strPrint);
};

#endif // VEJAMCRYPT_H
