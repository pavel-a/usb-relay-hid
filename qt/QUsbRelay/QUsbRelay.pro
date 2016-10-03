#-------------------------------------------------
#
# Project created by QtCreator 2016-09-25T13:01:44
#
#-------------------------------------------------

QT       -= core gui

TARGET = usb_relay_device
TEMPLATE = lib
CONFIG += dll

SOURCES += ../../lib/usb_relay_lib.c \
        ../../commandline/hiddata_mswin.c

HEADERS += ../../lib/usb_relay_device.h \
        ../../lib/usb_relay_hw.h \
        ../../commandline/hiddata.h


unix {
    target.path = /usr/lib
    INSTALLS += target
}

INCLUDEPATH += C:/Program Files (x86)/Windows Kits/8.0/Include/
INCLUDEPATH += ../../lib/usb-relay-dll/
INCLUDEPATH += ../../commandline/
