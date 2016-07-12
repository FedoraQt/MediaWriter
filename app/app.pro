TEMPLATE = app

TARGET = mediawriter

QT += qml quick widgets network

CONFIG += c++11

SOURCES += main.cpp \
    drivemanager.cpp \
    releasemanager.cpp \
    utilities.cpp

RESOURCES += qml.qrc \
    assets.qrc

include($$top_srcdir/deployment.pri)

target.path = $$BINDIR
INSTALLS += target

#DEFINES += HELPER_PATH=\\\"$${HELPER_PATH}\\\"

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
macx {
    SOURCES += macdrivemanager.cpp
}
