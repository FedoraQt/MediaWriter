CONFIG += link_pkgconfig
PKGCONFIG += adwaita-qt

TEMPLATE = app

TARGET = mediawriter

QT += qml quick quickcontrols2 widgets network

LIBS += -lisomd5

CONFIG += c++11

VERSION=4.2.50

HEADERS += \
    crashhandler.h \
    downloadmanager.h \
    drivemanager.h \
    fakedrivemanager.h \
    notifications.h \
    releasemanager.h \
    units.h \
    utilities.h \
    versionchecker.h \
    $$top_srcdir/theme/icon.h

SOURCES += main.cpp \
    crashhandler.cpp \
    downloadmanager.cpp \
    drivemanager.cpp \
    fakedrivemanager.cpp \
    releasemanager.cpp \
    notifications.cpp \
    units.cpp \
    utilities.cpp \
    versionchecker.cpp \
    $$top_srcdir/theme/icon.cpp

RESOURCES += qml.qrc \
    assets.qrc \
    ../translations/translations.qrc

lupdate_only {
    SOURCES += $$PWD/*.qml \
        $$PWD/complex/*.qml \
        $$PWD/dialogs/*.qml \
        $$PWD/simple/*.qml \
        $$PWD/views/*.qml \
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
    desktopfile.files = "$$top_srcdir/dist/linux/org.fedoraproject.MediaWriter.desktop"

    appdatafile.path = "$$DATADIR/metainfo/"
    appdatafile.files = "$$top_srcdir/dist/linux/org.fedoraproject.MediaWriter.appdata.xml"

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

    MY_ENTITLEMENTS.name = CODE_SIGN_ENTITLEMENTS
    MY_ENTITLEMENTS.value = "$$top_srcdir/app/Entitlements.plist"
    QMAKE_MAC_XCODE_SETTINGS += MY_ENTITLEMENTS

    QMAKE_POST_LINK += sed -i -e "s/@MEDIAWRITER_VERSION_SHORT@/$$MEDIAWRITER_VERSION_SHORT/g" \"./$${TARGET}.app/Contents/Info.plist\";
    QMAKE_POST_LINK += sed -i -e "s/@MEDIAWRITER_VERSION@/$$MEDIAWRITER_VERSION/g" \"./$${TARGET}.app/Contents/Info.plist\";
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
