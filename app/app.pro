TEMPLATE = app

TARGET = mediawriter

QT += qml quick widgets network

CONFIG += c++11

SOURCES += main.cpp \
    drivemanager.cpp \
    releasemanager.cpp \
    utilities.cpp \
    macdrivemanager.cpp

RESOURCES += qml.qrc \
    assets.qrc

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    drivemanager.h \
    releasemanager.h \
    utilities.h \
    linuxdrivemanager.h \
    macdrivemanager.h

linux {
    QT += dbus
    SOURCES += linuxdrivemanager.cpp
}
