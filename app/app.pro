TEMPLATE = app

TARGET = mediawriter

QT += qml quick widgets network

CONFIG += c++11

HEADERS += \
    drivemanager.h \
    releasemanager.h \
    utilities.h \
    fakedrivemanager.h \
    options.h

SOURCES += main.cpp \
    drivemanager.cpp \
    releasemanager.cpp \
    utilities.cpp \
    fakedrivemanager.cpp \
    options.cpp

RESOURCES += qml.qrc \
    assets.qrc

include($$top_srcdir/deployment.pri)

linux {
    target.path = $$BINDIR
    INSTALLS += target

    QT += dbus

    HEADERS += linuxdrivemanager.h
    SOURCES += linuxdrivemanager.cpp
}
macx {
    HEADERS += macdrivemanager.h
    SOURCES += macdrivemanager.cpp
}
win32 {
    HEADERS += windrivemanager.h
    SOURCES += windrivemanager.cpp
}
