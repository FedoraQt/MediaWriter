# this file is shared for all targets
# and it is actually located in the root folder of the project
unix {
    isEmpty(PREFIX) {
        PREFIX = /usr/local
    }
    BINDIR = $$PREFIX/bin
    DEFINES += BINDIR=\\\"$$BINDIR\\\"
    DATADIR = $$PREFIX/share
    DEFINES += DATADIR=\\\"$$DATADIR\\\"
    LIBEXECDIR = $$PREFIX/libexec/mediawriter
    DEFINES += LIBEXECDIR=\\\"$$LIBEXECDIR\\\"
}
