TEMPLATE = subdirs

linux {
    SUBDIRS = linux
}
win32 {
    SUBDIRS = win
}
macx {
    SUBDIRS = mac
}

lupdate_only {
    SUBDIRS = linux win mac
}
