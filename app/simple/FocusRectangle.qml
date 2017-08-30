import QtQuick 2.0

// TODO use a BorderImage or something else to draw a dashed rectangle, this is ugly
Rectangle {
    color: "transparent"
    radius: $(2)
    border {
        width: $(1)
        color: mixColors(palette.windowText, palette.button, 0.4)
    }
}
