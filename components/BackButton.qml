import QtQuick 2.3
import QtQuick.Controls 1.2

AdwaitaButton {
    width: arrow.width + text.width + $(28)
    Item {
        id: arrow
        anchors.left: parent.left
        anchors.leftMargin: $(12)
        anchors.verticalCenter: parent.verticalCenter
        rotation: -45
        transformOrigin: Item.Center
        width: $(10)
        height: $(10)
        Rectangle {
            x: $(1.5)
            y: $(1.5)
            width: $(2)
            height: $(9)
            radius: $(2)
            color: "#444444"
        }
        Rectangle {
            y: $(1.5)
            x: $(1.5)
            width: $(9)
            height: $(2)
            radius: $(2)
            color: "#444444"
        }
    }
    Text {
        id: text
        text: "Back"
        font.pixelSize: $(12)
        anchors {
            left: arrow.left
            leftMargin: $(16)
            verticalCenter: parent.verticalCenter
        }
    }
}
