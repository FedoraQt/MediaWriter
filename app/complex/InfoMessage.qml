import QtQuick 2.0
import QtQuick.Layouts 1.0

RowLayout {
    id: root
    property alias text: infoMessageText.text
    property bool error: false
    spacing: $(8)
    Rectangle {
        visible: !root.error
        Layout.fillWidth: false
        Layout.alignment: Qt.AlignVCenter
        width: $(18)
        height: $(18)
        radius: width / 2
        color: "#628fcf"
        border {
            width: $(1)
            color: "#a1a1a1"
        }
        Text {
            text: "i"
            anchors.centerIn: parent
            font.bold: true
            font.pointSize: $(9)
            color: "white"
        }
    }
    Rectangle {
        visible: root.error
        Layout.fillWidth: false
        Layout.alignment: Qt.AlignVCenter
        width: $(18)
        height: $(18)
        radius: width / 2
        color: "red"
        border {
            width: $(1)
            color: "#a1a1a1"
        }
        Rectangle {
            anchors.centerIn: parent
            width: parent.width * 0.65
            height: parent.width * 0.15
            color: "white"
        }
    }
    Text {
        id: infoMessageText
        Layout.fillHeight: true
        Layout.fillWidth: true
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.Wrap
        font.pointSize: $(9)
        color: palette.windowText
    }
}
