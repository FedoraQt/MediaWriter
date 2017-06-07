TEMPLATE = app

QT += core

LIBS += -lisomd5

CONFIG += c++11
CONFIG += console

TARGET = helper

include($$top_srcdir/deployment.pri)

target.path = $$LIBEXECDIR
INSTALLS += target

SOURCES = main.cpp \
    writejob.cpp \
    restorejob.cpp

HEADERS += \
    writejob.h \
    restorejob.h

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
