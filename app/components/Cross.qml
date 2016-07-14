import QtQuick 2.0

Item {
    width: childrenRect.width
    height: $(58)
    Rectangle {
        rotation: 45
        y: $(20)
        width: $(60)
        height: $(18)
        color: "red"
    }
    Rectangle {
        rotation: -45
        y: $(20)
        width: $(60)
        height: $(18)
        color: "red"
    }
}
