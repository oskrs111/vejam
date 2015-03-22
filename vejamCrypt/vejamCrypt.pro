#-------------------------------------------------
#
# Project created by QtCreator 2015-02-20T14:48:14
#
#-------------------------------------------------

QT       += core

QT       -= gui

#OpenSSL Libraries in Windows
INCLUDEPATH += "C:/OpenSSL-Win32/include"
LIBS += -L"C:\OpenSSL-Win32\lib\VC" -llibeay32MD
#OpenSSL Libraries in Windows - END

#OpenSSL Libraries in raspi-linux
#LIBS += -L"/mnt/rasp-pi-rootfs/usr/local/ssl/lib" -lcrypto
#OpenSSL Libraries in raspi-linux - END

#Qt Libraries in raspi-linux
#LIBS += -L"/mnt/rasp-pi-rootfs/usr/local/QtPi5.4.2/plugins/bearer" -lqconnmanbearer
#LIBS += -L"/mnt/rasp-pi-rootfs/usr/local/QtPi5.4.2/plugins/bearer" -lqgenericbearer
#Qt Libraries in raspi-linux - END


TARGET = vejamCryp
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    vejamcrypt.cpp

HEADERS += \
    vejamcrypt.h


