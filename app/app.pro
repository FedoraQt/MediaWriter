TEMPLATE = app

TARGET = mediawriter

QT += qml quick widgets network

CONFIG += c++11

HEADERS += \
    drivemanager.h \
    releasemanager.h \
    utilities.h \
    linuxdrivemanager.h \
    macdrivemanager.h \
    windrivemanager.h

SOURCES += main.cpp \
    drivemanager.cpp \
    releasemanager.cpp \
    utilities.cpp

RESOURCES += qml.qrc \
    assets.qrc

include($$top_srcdir/deployment.pri)

linux {
    target.path = $$BINDIR
    INSTALLS += target

    QT += dbus
    SOURCES += linuxdrivemanager.cpp
}
macx {
    SOURCES += macdrivemanager.cpp
}
win32 {
    SOURCES += windrivemanager.cpp
}
