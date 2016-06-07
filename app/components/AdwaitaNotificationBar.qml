import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.1

Rectangle {
    id: root
    clip: true

    color: "#729FCF"
    border {
        color: "#4C7BB2"
        width: 1
    }

    height: open ? $(70) : 0
    Behavior on height {
        NumberAnimation {
            duration: 300
            easing.type: Easing.OutElastic
            easing.amplitude: 0.7
            easing.period: 0.4
        }
    }

    property bool open: false

    property alias text: label.text
    property alias acceptText: buttonAccept.text
    property alias cancelText: buttonCancel.text

    signal accepted
    signal cancelled

    RowLayout {
        anchors {
            fill: parent
            margins: $(18)
        }
        spacing: $(12)
        Text {
            id: label

            Layout.fillWidth: true
            Layout.fillHeight: true
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere

            font.pixelSize: $(12)
            color: "white"
            textFormat: Text.RichText
        }

        AdwaitaButton {
            id: buttonCancel
            visible: text.length
            color: "transparent"

            onClicked: {
                root.open = false
                root.cancelled()
            }

            Layout.alignment: Qt.AlignVCenter
        }

        AdwaitaButton {
            id: buttonAccept
            visible: text.length

            onClicked: {
                root.open = false
                root.accepted()
            }

            Layout.alignment: Qt.AlignVCenter
        }
    }
}

