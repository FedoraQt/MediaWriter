import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.1
import QtQuick.Window 2.0

Item {
    id: popover
    z: -1
    property bool open: false
    visible: opacity > 0.0
    opacity: open ? 1.0 : 0.0
    Behavior on opacity { NumberAnimation { duration: 120 } }

    height: contents.height + $(12)
    width: contents.width + $(12)

    default property alias children: contents.data

    MouseArea {
        x: -mainWindow.widthf
        y: -mainWindow.height
        width: 2 * mainWindow.width
        height: 2 * mainWindow.height
        onClicked: {
            popover.open = false
        }
    }

    Rectangle {
        anchors.fill: contents
        anchors.margins: - $(12)
        color: palette.window
        antialiasing: true
        border {
            width: 1
            color: "#b1b1b1"
        }
        radius: $(6)
        Rectangle {
            z: -1
            y: -$(6.5) - 1
            antialiasing: true
            border.color: "#b1b1b1"
            border.width: 1
            color: palette.window
            anchors.horizontalCenter: parent.horizontalCenter
            width: $(14)
            height: $(14)
            rotation: 45
        }
        Rectangle {
            color: palette.window
            y: -$(6.5) + 1
            anchors.horizontalCenter: parent.horizontalCenter
            width: $(14)
            height: $(14)
            rotation: 45
        }
    }

    Item {
        id: contents
        width: childrenRect.width
        height: childrenRect.height
    }
}
