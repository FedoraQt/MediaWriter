import QtQuick 2.3

Item {
    width: childrenRect.width
    height: $(58)
    Rectangle {
        rotation: 45
        x: $(2)
        y: $(25)
        width: $(33)
        height: $(18)
        color: Qt.tint("light green", "green")
    }

    Rectangle {
        rotation: -45
        x: $(13)
        y: $(19)
        width: $(60)
        height: $(18)
        color: Qt.tint("light green", "green")
    }
}
