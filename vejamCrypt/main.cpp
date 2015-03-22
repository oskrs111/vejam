#include <QCoreApplication>
#include <QString>
#include <QDebug>
#include "vejamcrypt.h"

#include <openssl/des.h>
void test();
char * Encrypt( char *Key, char *Msg, int size);
char * Decrypt( char *Key, char *Msg, int size);


char* cls = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. In ornare ac arcu non molestie. Nam id augue aliquam, aliquam ante et, condimentum nunc. Nulla quis arcu vitae dolor iaculis vehicula efficitur in tellus. Quisque sed mi non ipsum bibendum pretium quis in nisi. Phasellus iaculis facilisis lorem nec tristique. Donec eleifend dui ut eleifend tempor. Etiam consectetur lorem mollis, venenatis dolor vel, vehicula magna. Integer condimentum erat ut lectus vehicula porttitor. Morbi neque ligula, interdum et commodo non, tincidunt eu nisi. Nullam rutrum faucibus neque, eu commodo diam. Sed vehicula leo nec dui maximus vulputate. Praesent ac urna accumsan, mattis libero in, ultricies mauris. Vivamus vitae magna eros. Vivamus risus lacus, pretium at massa in, dignissim dignissim justo. Suspendisse nisl magna, facilisis sed mauris et, vestibulum lobortis nulla. Morbi ullamcorper leo id urna consequat, sit amet venenatis eros blandit.";
char* pwd = "1234";

/*
EXPECTED RESULTS FOR:

char* cls = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. In ornare ac arcu non molestie. Nam id augue aliquam, aliquam ante et, condimentum nunc. Nulla quis arcu vitae dolor iaculis vehicula efficitur in tellus. Quisque sed mi non ipsum bibendum pretium quis in nisi. Phasellus iaculis facilisis lorem nec tristique. Donec eleifend dui ut eleifend tempor. Etiam consectetur lorem mollis, venenatis dolor vel, vehicula magna. Integer condimentum erat ut lectus vehicula porttitor. Morbi neque ligula, interdum et commodo non, tincidunt eu nisi. Nullam rutrum faucibus neque, eu commodo diam. Sed vehicula leo nec dui maximus vulputate. Praesent ac urna accumsan, mattis libero in, ultricies mauris. Vivamus vitae magna eros. Vivamus risus lacus, pretium at massa in, dignissim dignissim justo. Suspendisse nisl magna, facilisis sed mauris et, vestibulum lobortis nulla. Morbi ullamcorper leo id urna consequat, sit amet venenatis eros blandit.";
char* pwd = "1234";


ueYU+qy/uGFWuc0u5pan4IYODaJ0ZEztUoDr8781utbdCXr/4BdbmWZAL7JEgJP8q/CeCtNrMwRLfSekhxhpJCEI8lVjSkwWf7Hjf88V7JS/IS1L/EATe8T/mkXp
/UJQootYcxcK6S1uxbIYG09rRA59dGSJ+QrC4UcswjevFXtCMmas1zFPpQ2A8dOKsigUIsK0eqXMLko2AwRohDKcLRoFJrdezlR4TNq2wJ/k5vCXhYfVjPvPOnIM2zKlLWw
/XLslbhKG0QS9vqP0iV6QCnIim73jusiOKSb3KOXgiRSFyuMPoljNRNhe7RX4qclETC02MePbbPqx7OGf/rnoADt++e1NNOTQX5ykMDMUONnPGStEKO4MtrL7yeEX3qA
/dt1Q3yj2bVTAD2f0wFifSlShJi5PNqCBrVGRtUWA4NJz8PbZhJIL16BDuKW5ydRZym4wMUWqgZGLD9foIFlvzqMhPD9vaG94fbcNAirvoSGq6pso
/fkAfS9HCS9WOSdCCQIH1HDbtNeX2kuam3gS0+xFh/c0F0PXfhYENgcgImQuhtYKLeysf7QHx2TlkbUTDKvEu7OMa1wNlCJ56
/1W0rg2QndncQCcIYNYIkary6YkGVZi9DOvh8ZdKl3RTj8I4BbHQW1CNwbDzvf1IfqhQWX4ojfRjwWxLvO/9k6Th2cW3u6kimSE5KklSNsY3mXoQ7W3j
/lph62VuTWOma9TorOt9a7WrtZQvJOCatjZEm3WjSQoKZ90sW9wzd0HCjLM4oZfiSfEJgEKVQe65DOjaeIqsySz7fwQG7EjLY
/H29cbxj1A1NGe+x1KsF8U1bAZ6ohqm72OG+TXuOXHh6fKFfoNNVUaVrmO8lp00gbOGueHIG9EvFxcUgz9S6nl61E8vOH8Aa36BBLWxh4qaflqacFQpWmkxhJ2x7c769pYSlydcDHV8nYG6VsvSCFEgvfh1or
/itimjjbUAcwQMsxqldpszPgL3dPzuCA/V+zei2HFZda0WJTO7xjXMcJBnReKpTlKQeA9Mlk+4FMEmD1h+c37wwzr0tOY97x2EjBmS24RTMfQMODK7R9HEtQmIaru24cEDCQfY1tR0SFkB+3xKFp3uKOzc34B2eqNJ8FN
/NFkUwgRF3atu1BCOos18Yd
/8RsD4eOHVYRubj50t2ecy7Ir8+DcB6T7rHzNfs8Cj7PZC98FT3xIBP6dmToX6TeNZHAchrS92OkFCSji+DAhN89qkV0l/LkMZmHcER6ijT4=

*/


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QString r;
	QString s;
    vejamCrypt cr;
    cr.vcSetEncryptMode(vejamCrypt::vcDES_ECB);

    qDebug() << cls << "\r\n";

    r = cr.vcEncryptB64(QString(cls),pwd);
	
    qDebug() << r << "\r\n";

	s = cr.vcDecryptB64(r,pwd);	

    qDebug() << s << "\r\n";


    return a.exec();
}
