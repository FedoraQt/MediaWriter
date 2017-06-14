TEMPLATE = app

QT += core

LIBS += -lcheckisomd5 -limplantisomd5 -liso9660io
PKGCONFIG += isomd5sum iso9660io

CONFIG += c++11
CONFIG += console
CONFIG += link_pkgconfig

TARGET = helper

include($$top_srcdir/deployment.pri)

target.path = $$LIBEXECDIR
INSTALLS += target

SOURCES = main.cpp write.cpp

HEADERS += write.h

linux {
    include(linux/linux.pri)
}
win32 {
    include(win/win.pri)
}
macx {
    include(mac/mac.pri)
}

RESOURCES += ../translations/translations.qrc
