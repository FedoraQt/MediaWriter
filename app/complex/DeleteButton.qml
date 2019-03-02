import QtQuick 2.0
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.0

import "../simple"

RowLayout {
    id: deleteButtonRoot

    property string errorText: ""
    signal started

    states: [
        State {
            name: "hidden"
            PropertyChanges {
                target: eraseButton
                opacity: 0.0
            }
        },
        State {
            name: "ready"
            PropertyChanges {
                target: eraseButton
                enabled: true
                text: qsTr("Delete the Downloaded Image")
            }
        },
        State {
            name: "working"
            PropertyChanges {
                target: eraseIndicator
                opacity: 1.0
            }
        },
        State {
            name: "success"
            PropertyChanges {
                target: eraseCheckmark
                opacity: 1.0
            }
            PropertyChanges {
                target: hideTimer
                running: true
            }
        },
        State {
            name: "error"
            PropertyChanges {
                target: eraseCross
                opacity: 1.0
            }
            PropertyChanges {
                target: eraseText
                text: deleteButtonRoot.errorText
            }
        }
    ]

    Timer {
        id: workTimer
        interval: 800
        onTriggered: {
            started()
        }
    }

    Timer {
        id: hideTimer
        interval: 5000
        onTriggered: {
            deleteButtonRoot.state = "hidden"
        }
    }

    Text {
        id: eraseText
        Layout.alignment: Qt.AlignRight
        Layout.maximumWidth: parent.width - eraseButton.width - parent.spacing
        Layout.fillHeight: true
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignRight
        font.pointSize: $$(9)
        textFormat: Text.RichText
        color: palette.windowText
        onLinkActivated: Qt.openUrlExternally("file://" + link)
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        Behavior on opacity { NumberAnimation { duration: 120 } }
    }
    AdwaitaButton {
        id: eraseButton
        Layout.alignment: Qt.AlignRight
        enabled: false
        visible: opacity > 0.0
        opacity: 1.0
        color: "red"
        textColor: "white"
        text: ""
        Behavior on opacity { NumberAnimation { duration: 240 } }
        Behavior on implicitWidth { NumberAnimation { duration: deleteButtonRoot.state == "working" ? 120 : 240 } }
        onClicked: {
            deleteButtonRoot.state = "working"
            workTimer.start()
        }
        BusyIndicator {
            id: eraseIndicator
            anchors.fill: parent
            anchors.margins: $(9)
            opacity: 0.0
            visible: opacity > 0.0
            Behavior on opacity { NumberAnimation { duration: 120 } }
        }
        CheckMark {
            id: eraseCheckmark
            anchors.fill: parent
            anchors.margins: $(9)
            opacity: 0.0
            visible: opacity > 0.0
            Behavior on opacity { NumberAnimation { duration: 120 } }
        }
        Cross {
            id: eraseCross
            anchors.fill: parent
            anchors.margins: $(9)
            opacity: 0.0
            visible: opacity > 0.0
            Behavior on opacity { NumberAnimation { duration: 120 } }
        }
    }
}
