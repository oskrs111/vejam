#include <QDebug>
#include <openssl/des.h>
#include "vejamcrypt.h"

//Based on:
//http://blog.fpmurphy.com/2010/04/openssl-des-api.html
//http://www.codealias.info/technotes/des_encryption_using_openssl_a_simple_example

vejamCrypt::vejamCrypt()
{
}

void vejamCrypt::vcSetEncryptMode(int encryptMode)
{
    this->m_Mode = encryptMode;
}

QString vejamCrypt::vcEncryptB64(QString toEncrypt, QString password)
{
	QByteArray r;
	r = vcEncrypt(toEncrypt, password);
	r.append("\0\0\0\0\0\0\0\0");
    return QString(r.toBase64());
}

QString vejamCrypt::vcDecryptB64(QString toDecrypt, QString password)
{
    QByteArray decrypt;
	decrypt = QByteArray::fromBase64(toDecrypt.toLatin1());
	return this->vcDecrypt(decrypt, password);	 
}

QByteArray vejamCrypt::vcEncrypt(QString toEncrypt, QString password)
{
    switch(this->m_Mode)
    {
        case vcDES_ECB:
             return vejamCrypt::vcDES_Encrypt(toEncrypt, password);
             break;

        case vcTripleDES:
        case vcAES:
        case vcLast:
        case vcUndefined:
        default: break;
    }
    return "";
}

QString vejamCrypt::vcDecrypt(QByteArray toDecrypt, QString password)
{
        switch(this->m_Mode)
    {
        case vcDES_ECB:
             return vejamCrypt::vcDES_Decrypt(toDecrypt, password);
             break;

        case vcTripleDES:
        case vcAES:
        case vcLast:
        case vcUndefined:
        default: break;
    }
    return "";	
}

QByteArray vejamCrypt::vcDES_Encrypt(QString toEncrypt, QString password)
{
    QByteArray dataOut, m;
    unsigned char* in = 0;
    unsigned char* out = 0;
	int n = 0;
	int t = 0;
	int cycles = 0;

    DES_cblock key;    
    DES_key_schedule schedule;

	//64Bit blocks! 
	cycles = (toEncrypt.size()/8); 

	//Check if last block needs to be padded
	n = toEncrypt.size() - (cycles * 8);	
	if((n != 0) && (n < 8))
	//Padding with spaces as 'Paul Tero, JS library' does!
	{
		QByteArray padd;
		padd.fill(' ',(8 - n));
		toEncrypt.append(padd);
		cycles++;
	}
	
	toEncrypt.append("\0\0\0\0\0\0\0\0");
	m = toEncrypt.toLocal8Bit();
	in = (unsigned char*)m.constData();
	//vcHexDump(in, "toEncrypt",true);

    dataOut.resize(toEncrypt.size()*4);
    dataOut.fill(0x00);
    out = (unsigned char*)dataOut.data();

    memset(&key, 0x00, sizeof(DES_cblock));
    if(password.size() < sizeof(DES_cblock))
    {
		memcpy(&key, (unsigned char*)password.toLocal8Bit().data(), password.size());
    }
    else
    {
        memcpy(&key, (unsigned char*)password.toLocal8Bit().data(), sizeof(DES_cblock));
    }
    	
	DES_set_odd_parity( &key );	
	
	DES_set_key((C_Block *)key, &schedule);

	for(t=0; t<cycles; t++)
	{
		DES_ecb_encrypt((C_Block *)in,(C_Block *)out, &schedule, DES_ENCRYPT);	
		in += 8;
		out += 8;
	}	
	
	//Set the rigth size for output Buffer...	
	dataOut.resize(t*8);
	out = (unsigned char*)dataOut.data();
	//vcHexDump(out, "Encrypted", false);
	return dataOut;
}

QString vejamCrypt::vcDES_Decrypt(QByteArray toDecrypt, QString password)
{  
    QByteArray dataOut;
	unsigned char* in = 0;
    unsigned char* out = 0;
	int t = 0;
	int cycles = 0;

    DES_cblock key;    	
    DES_key_schedule schedule;	

	//64Bit blocks! 
	cycles = (toDecrypt.size()/8); 

	in = (unsigned char*)toDecrypt.constData();
	//vcHexDump(in, "toDecrypt", false);

    dataOut.resize(toDecrypt.size()*2);
    dataOut.fill(0x00);
    out = (unsigned char*)dataOut.data();

    memset(&key, 0x00, sizeof(DES_cblock));
    if(password.size() < sizeof(DES_cblock))
    {
		memcpy(&key, (unsigned char*)password.toLocal8Bit().data(), password.size());
    }
    else
    {
        memcpy(&key, (unsigned char*)password.toLocal8Bit().data(), sizeof(DES_cblock));
    }
    	
	DES_set_odd_parity( &key );		
	DES_set_key((C_Block *)key, &schedule);	
	
	
	for(t=0; t<cycles; t++)
	{
		DES_ecb_encrypt((C_Block *)in,(C_Block *)out, &schedule, DES_DECRYPT);			
		in += 8;
		out +=8;
	}
			
	out = (unsigned char*)dataOut.data();
	//vcHexDump(out, "Decrypted", true);
	return dataOut;
}

void vejamCrypt::vcHexDump(unsigned char* d, char* ref, bool strPrint)
{
	int lock = 0;
	int lf = 0;
	unsigned char* dd = d;
	if(ref == 0) ref = "*";
	if(d == 0) return;

	printf("\r\nHex Dump[%s]:\r\n",ref);
	while (*d)
	{
		printf("%02X ", *d++);
		lock++;
		lf++;
		if(lock > VC_HEX_DUMP_MAX) break;
		if(lf == 32)
		{
			lf = 0;
			printf("\r\n");
		}
	}

	if(strPrint)
	{
		lock = 0;
		lf = 0;	
		printf("\r\n\r\n");
		while (*dd)
		{
			printf("%c", *dd++);
			lock++;
			lf++;
			if(lock > VC_HEX_DUMP_MAX) break;
			if(lf == 96)
			{
				lf = 0;
				printf("\r\n");
			}
		}
	}
	

	printf("\r\n\r\n");
}
