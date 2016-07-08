isEmpty(target.path) {
    target.path = $${PREFIX}/usr/bin
    export(target.path)
}
INSTALLS += target

export(INSTALLS)
