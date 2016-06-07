import QtQuick 2.3

Item {
    implicitHeight: 10
    width: height / 2
    clip: true
    scale: $(1)
    property alias color: rect.color
    Rectangle {
        id: rect
        x: -parent.height / 5 * 4
        y: -parent.height / 10
        rotation: 45
        width: parent.height
        height: parent.height
        color: "black"
    }
}
