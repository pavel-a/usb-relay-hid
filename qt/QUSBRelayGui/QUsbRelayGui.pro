#-------------------------------------------------
#
# Project created by QtCreator 2016-10-02T19:08:35
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QUsbRelayGui
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    device.cpp

HEADERS  += widget.h \
    device.h

FORMS    += widget.ui \
    device.ui

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../QUsbRelay/release/ -lusb_relay_device
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../QUsbRelay/debug/ -lusb_relay_device
else:unix: LIBS += -L$$OUT_PWD/../QUsbRelay/ -lusb_relay_device

INCLUDEPATH += $$PWD/../QUsbRelay
DEPENDPATH += $$PWD/../QUsbRelay

INCLUDEPATH += $$PWD/../../lib
DEPENDPATH += $$PWD/../../lib

RESOURCES += \
    resources.qrc

win32: RC_ICONS = device_and_hardware_w.ico
