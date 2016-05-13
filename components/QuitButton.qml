import QtQuick 2.3
import QtQuick.Controls 1.2

AdwaitaButton {
    Item {
        anchors.fill: parent
        rotation: 45
        transformOrigin: Item.Center
        Rectangle {
            width: $(2)
            height: $(12)
            radius: $(1)
            anchors.centerIn: parent
            color: "#a1a1a1"
        }
        Rectangle {
            width: $(12)
            height: $(2)
            radius: $(1)
            anchors.centerIn: parent
            color: "#a1a1a1"
        }
    }
}
