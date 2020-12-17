TEMPLATE = subdirs

SUBDIRS = lib app helper theme

app.depends = lib theme
helper.depends = lib

# Generate translations
TRANSLATIONS_BUILD_RESULT = $$system(pushd translations > /dev/null; \
                                     ./build-translations.sh > /dev/null; \
                                     popd >/dev/null; \
                                     echo $?)
                                     
!eval($$TRANSLATIONS_BUILD_RESULT == "0") {
    message("Failed to generate translations")
}

equals(QT_MAJOR_VERSION, 5):lessThan(QT_MINOR_VERSION, 12) {
    message("Cannot use Qt $${QT_VERSION}")
    error("Qt 5.12 and newer is required")
}
