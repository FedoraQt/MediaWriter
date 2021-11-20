/*
 * Fedora Media Writer
 * Copyright (C) 2016 Martin Bříza <mbriza@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.12
import QtQuick.Controls 2.12 as QQC2
//import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.12

import MediaWriter 1.0

QQC2.Dialog {
    id: root
    title: drives.lastRestoreable ? qsTr("Restore %1?").arg(drives.lastRestoreable.name) : ""

    Connections {
        target: drives
        function onLastRestoreableChanged() {
            if (drives.lastRestoreable == null)
                root.close()
        }
    }

    contentItem : Rectangle {
        focus: true
        implicitWidth: 480
        implicitHeight: textItem.height + buttonItem.height + Math.round(units.gridUnit * 2.5)
        height: textItem.height + buttonItem.height + Math.round(units.gridUnit * 2.5)
        color: palette.window

        Keys.onEscapePressed: {
            if (drives.lastRestoreable.restoreStatus != Drive.RESTORING)
                root.close()
        }

        Item {
            id: wrapper
            anchors.fill: parent
            anchors.margins: units.gridUnit
            Row {
                id: textItem
                spacing: units.gridUnit * 2
                x: !drives.lastRestoreable || drives.lastRestoreable.restoreStatus == Drive.CONTAINS_LIVE ? 0 :
                                              drives.lastRestoreable.restoreStatus == Drive.RESTORING     ? - (parent.width + (units.gridUnit * 2)) :
                                                                                                            - (2 * parent.width + (units.gridUnit * 4))
                height: warningText.height
                Behavior on x {
                    NumberAnimation {
                        duration: 300
                        easing.type: Easing.OutExpo
                    }
                }
                QQC2.Label {
                    id: warningText
                    width: wrapper.width
                    text: qsTr( "<p align=\"justify\">
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
                }
                ColumnLayout {
                    id: progress
                    width: wrapper.width
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: units.largeSpacing + units.smallSpacing
                    Item {
                        width: 1; height: 1
                    }

                    QQC2.ProgressBar {
                        id: progressIndicator
                        width: units.gridUnit * 14
                        Layout.alignment: Qt.AlignHCenter
                        indeterminate: true
                    }

                    QQC2.Label {
                        Layout.alignment: Qt.AlignHCenter
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                        text: qsTr("<p align=\"justify\">Please wait while Fedora Media Writer restores your portable drive.</p>")
                    }
                }
                ColumnLayout {
                    visible: drives.lastRestoreable && drives.lastRestoreable.restoreStatus != Drive.RESTORE_ERROR
                    width: wrapper.width
                    anchors.verticalCenter: parent.verticalCenter
                    Icon {
                        Layout.alignment: Qt.AlignHCenter
                        height: 64
                        width: 64
                        source: "qrc:/icons/checkmark"
                    }
                    QQC2.Label {
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: qsTr("Your drive was successfully restored!")
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    }
                }
                ColumnLayout {
                    visible: drives.lastRestoreable && drives.lastRestoreable.restoreStatus != Drive.RESTORED
                    width: wrapper.width
                    anchors.verticalCenter: parent.verticalCenter
                    Icon {
                        Layout.alignment: Qt.AlignHCenter
                        height: 64
                        width: 64
                        source: "qrc:/icons/window-close"
                    }
                    QQC2.Label {
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: qsTr("Unfortunately, an error occurred during the process.<br>Please try restoring the drive using your system tools.")
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    }
                }
            }

            RowLayout {
                id: buttonItem
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                spacing: units.largeSpacing + units.smallSpacing
                QQC2.Button {
                    text: qsTr("Cancel")
                    visible: drives.lastRestoreable &&
                             drives.lastRestoreable.restoreStatus != Drive.RESTORED &&
                             drives.lastRestoreable.restoreStatus != Drive.RESTORE_ERROR ? true : false
                    Behavior on x { NumberAnimation {} }
                    onClicked: root.visible = false
                }
                QQC2.Button {
                    id: restoreButton
                    text: drives.lastRestoreable && drives.lastRestoreable.restoreStatus == Drive.CONTAINS_LIVE ? qsTr("Restore") : qsTr("Close")
                    highlighted: true
                    enabled: !drives.lastRestoreable || drives.lastRestoreable.restoreStatus != Drive.RESTORING
                    onClicked: {
                        if (drives.lastRestoreable && drives.lastRestoreable.restoreStatus == Drive.CONTAINS_LIVE)
                            drives.lastRestoreable.restore()
                        else
                            root.visible = false
                    }
                    onTextChanged: {
                        if (restoreButton.hasOwnProperty("destructiveAction")) {
                            restoreButton.destructiveAction = restoreButton.text == qsTr("Restore")
                        }
                    }
                }
            }
        }
    }
}
