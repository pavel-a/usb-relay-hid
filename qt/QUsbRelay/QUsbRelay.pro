QT  -= core gui

TARGET = usb_relay_device
TEMPLATE = lib
CONFIG += dll

SOURCES += ../../lib/usb_relay_lib.c
win32{
    SOURCES += ../../commandline/hiddata_mswin.c
}
unix{
    SOURCES += ../../commandline/hiddata_libusb01.c
}

HEADERS += ../../lib/usb_relay_device.h \
        ../../lib/usb_relay_hw.h \
        ../../commandline/hiddata.h

INCLUDEPATH += ../../lib/usb-relay-dll/
INCLUDEPATH += ../../commandline/

win32{
    INCLUDEPATH += C:/Program Files (x86)/Windows Kits/8.0/Include/
}

unix:{
CONFIG += link_pkgconfig
PKGCONFIG += libusb
}
