TEMPLATE = app

TARGET = mediawriter

QT += qml quick widgets network

LIBS += -lisomd5

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
    assets.qrc \
    ../translations/translations.qrc

lupdate_only {
    SOURCES += $$PWD/*.qml \
        $$PWD/components/*.qml
}

include($$top_srcdir/deployment.pri)

target.path = $$BINDIR
INSTALLS += target

linux {
    QT += dbus

    HEADERS += linuxdrivemanager.h
    SOURCES += linuxdrivemanager.cpp

    icon.path = "$$DATADIR/icons/hicolor/192x192/"
    icon.files = assets/mediawriter.png

    desktopfile.path = "$$DATADIR/applications/"
    desktopfile.files = "$$top_srcdir/dist/linux/mediawriter.desktop"

    appdatafile.path = "$$DATADIR/appdata/"
    appdatafile.files = "$$top_srcdir/dist/linux/mediawriter.appdata.xml"

    INSTALLS += icon desktopfile appdatafile
}
macx {
    HEADERS += macdrivemanager.h \
                macdrivearbiter.h
    SOURCES += macdrivemanager.cpp
    OBJECTIVE_SOURCES += macdrivearbiter.mm

    QMAKE_LFLAGS += -F/System/Library/Frameworks
    LIBS += -framework Foundation
    LIBS += -framework DiskArbitration
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9

    ICON = assets/mediawriter.icns
}
win32 {
    HEADERS += windrivemanager.h
    SOURCES += windrivemanager.cpp

    # Until I find out how (or if it's even possible at all) to run a privileged process from an unprivileged one, the main binary will be privileged too
    DISTFILES += windows.manifest
    QMAKE_MANIFEST = $${PWD}/windows.manifest

    RC_ICONS = assets/mediawriter.ico
}
