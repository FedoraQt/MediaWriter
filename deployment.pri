# this file is shared for all targets
# and it is actually located in the root folder of the project
isEmpty(PREFIX) {
    PREFIX = /usr/local
}
BINDIR = $$PREFIX/bin
DATADIR = $$PREFIX/share
isEmpty(LIBEXECDIR) {
    LIBEXECDIR = $$PREFIX/libexec/mediawriter
}
linux {
    DEFINES += BINDIR=\\\"$$BINDIR\\\"
    DEFINES += DATADIR=\\\"$$DATADIR\\\"
    DEFINES += LIBEXECDIR=\\\"$$LIBEXECDIR\\\"
}
QMAKE_LIBDIR += $$top_builddir/lib
INCLUDEPATH += $$top_srcdir/lib/ $$top_srcdir/theme/
isEmpty(MEDIAWRITER_VERSION) {
    MEDIAWRITER_VERSION=$$quote($$system(git -C \""$$_PRO_FILE_PWD_"\" describe --tags || echo "Not-Available"))
}
MEDIAWRITER_VERSION_SHORT=$$section(MEDIAWRITER_VERSION,-,0,0)
DEFINES += MEDIAWRITER_VERSION="\\\"$$MEDIAWRITER_VERSION\\\""
