import QtQuick 2.4
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
        width: $(17)
        height: $(17)
        radius: width / 2
        color: "#628fcf"
        border {
            width: $(1)
            color: "#a1a1a1"
        }
        Rectangle {
            width: $(1)
            height: $(6)
            anchors {
                bottom: parent.bottom
                bottomMargin: (parent.height - height) / 3
                horizontalCenter: parent.horizontalCenter
            }
            color: "#cce3e3e3"
        }
        Rectangle {
            width: $(1)
            height: $(1)
            anchors {
                top: parent.top
                topMargin: (parent.height - height) / 4
                horizontalCenter: parent.horizontalCenter
            }
            color: "#cce3e3e3"
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
        textFormat: Text.RichText
        font.pointSize: $$(9)
        color: palette.windowText
        onLinkActivated: Qt.openUrlExternally(link)
        MouseArea {
            acceptedButtons: Qt.NoButton
            anchors.fill: parent
            cursorShape: infoMessageText.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
        }
    }
}
