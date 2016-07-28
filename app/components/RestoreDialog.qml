import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Window 2.0
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1

import MediaWriter 1.0

Dialog {
    id: root
    title: drives.lastRestoreable ? qsTranslate("", "Restore %1?").arg(drives.lastRestoreable.name) : ""

    Connections {
        target: drives
        onLastRestoreableChanged: {
            if (drives.lastRestoreable == null)
                root.close()
        }
    }

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
                x: !drives.lastRestoreable || drives.lastRestoreable.restoreStatus == Drive.CONTAINS_LIVE ? 0 :
                                              drives.lastRestoreable.restoreStatus == Drive.RESTORING     ? - (parent.width + $(36)) :
                                                                                                            - (2 * parent.width + $(72))
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
                    visible: drives.lastRestoreable && drives.lastRestoreable.restoreStatus != Drive.RESTORE_ERROR
                    width: wrapper.width
                    anchors.verticalCenter: parent.verticalCenter
                    CheckMark {
                        Layout.alignment: Qt.AlignHCenter
                    }
                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: qsTranslate("", "Your drive was successfully restored!")
                        font.pixelSize: $(12)
                    }
                }
                ColumnLayout {
                    visible: drives.lastRestoreable && drives.lastRestoreable.restoreStatus != Drive.RESTORED
                    width: wrapper.width
                    anchors.verticalCenter: parent.verticalCenter
                    Cross {
                        Layout.alignment: Qt.AlignHCenter
                    }
                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: qsTranslate("", "Unfortunately, an error occured during the process.<br>Please try restoring the drive using your system tools.")
                        font.pixelSize: $(12)
                    }
                }
            }

            Row {
                id: buttonItem
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                spacing: $(12)
                /*
                AdwaitaButton {
                    text: qsTranslate("", "Cancel")
                    enabled: drives.lastRestoreable.restoreStatus != Drive.RESTORING
                    visible: opacity > 0.0
                    opacity: root.state == 2 ? 0.0 : 1.0
                    Behavior on opacity { NumberAnimation {} }
                    Behavior on x { NumberAnimation {} }
                    onClicked: root.visible = false
                }
                */
                AdwaitaButton {
                    text: drives.lastRestoreable && drives.lastRestoreable.restoreStatus == Drive.CONTAINS_LIVE ? qsTranslate("", "Restore") : qsTranslate("", "Close")
                    color: drives.lastRestoreable && drives.lastRestoreable.restoreStatus == Drive.CONTAINS_LIVE ? "red" : "#628fcf"
                    textColor: "white"
                    enabled: !drives.lastRestoreable || drives.lastRestoreable.restoreStatus != Drive.RESTORING
                    onClicked: {
                        if (drives.lastRestoreable && drives.lastRestoreable.restoreStatus == Drive.CONTAINS_LIVE)
                            drives.lastRestoreable.restore()
                        else
                            root.visible = false
                    }
                }
            }
        }
    }
}
