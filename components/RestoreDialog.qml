import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Window 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1

Dialog {
    id: root
    title: drives.lastRestoreableDrive ? qsTranslate("", "Restore %1?").arg(liveUSBData.driveToRestore.text) : ""

    property int state: 0

    Connections {
        target: drives.lastRestoreable
        onBeingRestoredChanged: root.state++
    }

    onVisibleChanged: state = 0

    contentItem : Rectangle {
        implicitWidth: $(480)
        implicitHeight: textItem.height + buttonItem.height + $(48)
        height: textItem.height + buttonItem.height + $(48)
        color: palette.window
        Item {
            id: wrapper
            anchors.fill: parent
            anchors.margins: $(18)
            Row {
                id: textItem
                spacing: $(36)
                x: root.state == 0 ? 0 : root.state == 1 ? - (parent.width + $(36)) : - (2 * parent.width + $(72))
                height: warningText.height
                Behavior on x {
                    NumberAnimation {
                        duration: 300
                        easing.type: Easing.OutExpo
                    }
                }
                Text {
                    id: warningText
                    width: wrapper.width
                    text: qsTranslate("",  "<p align=\"justify\">
                                                To reclaim all space available on the drive, it has to be restored to its factory settings.
                                                The live system and all saved data will be deleted.
                                            </p>
                                            <p align=\"justify\">
                                                You don't need to restore the drive if you want to write another live system to it.
                                            </p>
                                            <p align=\"justify\">
                                                Do you want to continue?
                                            </p>")
                    textFormat: Text.RichText
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    font.pixelSize: $(12)
                }
                ColumnLayout {
                    id: progress
                    width: wrapper.width
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: $(12)
                    Item {
                        width: 1; height: 1
                    }

                    AdwaitaBusyIndicator {
                        id: progressIndicator
                        width: $(256)
                        Layout.alignment: Qt.AlignHCenter
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                        text: qsTranslate("", "<p align=\"justify\">Please wait while Fedora Media Writer restores your portable drive.</p>")
                        font.pixelSize: $(12)
                    }
                }
                ColumnLayout {
                    width: wrapper.width
                    anchors.verticalCenter: parent.verticalCenter
                    CheckMark {
                        Layout.alignment: Qt.AlignHCenter
                    }
                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTranslate("", "Your drive was successfully restored!")
                        font.pixelSize: $(12)
                    }
                }
            }

            Row {
                id: buttonItem
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                spacing: $(12)
                AdwaitaButton {
                    text: qsTranslate("", "Cancel")
                    enabled: root.state == 0
                    visible: opacity > 0.0
                    opacity: root.state == 2 ? 0.0 : 1.0
                    Behavior on opacity { NumberAnimation {} }
                    Behavior on x { NumberAnimation {} }
                    onClicked: root.visible = false
                }
                AdwaitaButton {
                    text: root.state == 2 ? qsTranslate("", "Close") : qsTranslate("", "Restore")
                    color: root.state == 2 ? "#628fcf" : "red"
                    textColor: "white"
                    enabled: root.state != 1
                    onClicked: {
                        if (root.state == 0)
                            liveUSBData.driveToRestore.restore()
                        else
                            root.visible = false
                    }
                }
            }
        }
    }
}
