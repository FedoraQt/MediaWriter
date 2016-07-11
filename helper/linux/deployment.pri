isEmpty(target.path) {
    target.path = $${PREFIX}/usr/libexec/mediawriter
    export(target.path)
}
INSTALLS += target

export(INSTALLS)
