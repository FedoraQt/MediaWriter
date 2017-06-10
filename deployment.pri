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
isEmpty(MEDIAWRITER_VERSION) {
    DEFINES += MEDIAWRITER_VERSION="\\\"$(shell git -C \""$$_PRO_FILE_PWD_"\" describe --tags || echo N/A)\\\""
} else {
    DEFINES += MEDIAWRITER_VERSION="\\\"$$MEDIAWRITER_VERSION\\\""
}
