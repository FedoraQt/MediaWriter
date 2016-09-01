TEMPLATE = subdirs

SUBDIRS = lib app helper

app.depends = lib
helper.depends = lib
