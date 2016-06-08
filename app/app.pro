TEMPLATE = app

QT += qml quick widgets network

CONFIG += c++11

SOURCES += main.cpp \
    drivemanager.cpp \
    releasemanager.cpp \
    utilities.cpp

RESOURCES += qml.qrc \
    assets.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    drivemanager.h \
    releasemanager.h \
    utilities.h \
    linuxdrivemanager.h

linux {
    QT += dbus
    SOURCES += linuxdrivemanager.cpp
}
