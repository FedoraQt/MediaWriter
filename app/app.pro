TEMPLATE = app

TARGET = mediawriter

QT += qml quick widgets network

LIBS += -lcheckisomd5
PKGCONFIG += isomd5sum

CONFIG += c++11
CONFIG += link_pkgconfig

HEADERS += \
    drivemanager.h \
    releasemanager.h \
    utilities.h \
    fakedrivemanager.h \
    notifications.h \
    crashhandler.h \
    downloadmanager.h \
    versionchecker.h

SOURCES += main.cpp \
    drivemanager.cpp \
    releasemanager.cpp \
    utilities.cpp \
    fakedrivemanager.cpp \
    notifications.cpp \
    crashhandler.cpp \
    downloadmanager.cpp \
    versionchecker.cpp

RESOURCES += qml.qrc \
    assets.qrc \
    ../translations/translations.qrc

lupdate_only {
    SOURCES += $$PWD/*.qml \
        $$PWD/components/*.qml \
        linuxdrivemanager.cpp \
        macdrivemanager.cpp \
        windrivemanager.cpp
    HEADERS += linuxdrivemanager.h \
        macdrivemanager.h \
        windrivemanager.h
}

include($$top_srcdir/deployment.pri)

target.path = $$BINDIR
INSTALLS += target

linux {
    QT += dbus

    HEADERS += linuxdrivemanager.h
    SOURCES += linuxdrivemanager.cpp

    icon.path = "$$DATADIR/icons/hicolor"
    icon.files = assets/icon/16x16 \
                 assets/icon/22x22 \
                 assets/icon/24x24 \
                 assets/icon/32x32 \
                 assets/icon/48x48 \
                 assets/icon/256x256 \
                 assets/icon/512x512

    desktopfile.path = "$$DATADIR/applications/"
    desktopfile.files = "$$top_srcdir/dist/linux/mediawriter.desktop"

    appdatafile.path = "$$DATADIR/appdata/"
    appdatafile.files = "$$top_srcdir/dist/linux/mediawriter.appdata.xml"

    INSTALLS += icon desktopfile appdatafile
}
macx {
    TARGET = "Fedora Media Writer"
    HEADERS += macdrivemanager.h \
                macdrivearbiter.h
    SOURCES += macdrivemanager.cpp
    OBJECTIVE_SOURCES += macdrivearbiter.mm

    QMAKE_LFLAGS += -F/System/Library/Frameworks
    LIBS += -framework Foundation
    LIBS += -framework DiskArbitration
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9

    QMAKE_INFO_PLIST = Info.plist
    ICON = assets/icon/mediawriter.icns
}
win32 {
    HEADERS += windrivemanager.h
    SOURCES += windrivemanager.cpp
    RESOURCES += windowsicon.qrc

    LIBS += -ldbghelp

    # Until I find out how (or if it's even possible at all) to run a privileged process from an unprivileged one, the main binary will be privileged too
    DISTFILES += windows.manifest
    QMAKE_MANIFEST = $${PWD}/windows.manifest

    RC_ICONS = assets/icon/mediawriter.ico
    static {
        QTPLUGIN.platforms = qwindows
    }
}
