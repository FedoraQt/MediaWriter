isEmpty(target.path) {
    target.path = $${PREFIX}/usr/libexec/mediawriter
    export(target.path)
}
INSTALLS += target

HELPER_PATH = $${PREFIX}/usr/libexec/mediawriter/helper
export(HELPER_PATH)

export(INSTALLS)
