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
//import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.12
import QtQuick.Window 2.12

import MediaWriter 1.0

QQC2.Dialog {
    id: dialog
    title: qsTr("Write %1").arg(releases.selected.name)

    height: layout.height + (units.gridUnit * 2)
    standardButtons: StandardButton.NoButton

    width: 640

    function reset() {
        writeArrow.color = palette.text
    }

    onVisibleChanged: {
        if (!visible) {
            if (drives.selected)
                drives.selected.cancel()
            releases.variant.resetStatus()
            downloadManager.cancel()
        }
        reset()
    }

    Connections {
        target: releases
        function onSelectedChanged() {
            reset();
        }
    }

    Connections {
        target: drives
        function onSelectedChanged() {
            drives.selected.delayedWrite = false
        }
    }

    contentItem: Rectangle {
        id: dialogContainer
        anchors.fill: parent
        color: palette.window
        focus: true

        states: [
            State {
                name: "preparing"
                when: releases.variant.status === Variant.PREPARING
            },
            State {
                name: "downloading"
                when: releases.variant.status === Variant.DOWNLOADING
                PropertyChanges {
                    target: messageDownload
                    visible: true
                }
                PropertyChanges {
                    target: progressBar;
                    value: releases.variant.progress.ratio
                }
            },
            State {
                name: "download_verifying"
                when: releases.variant.status === Variant.DOWNLOAD_VERIFYING
                PropertyChanges {
                    target: messageDownload
                    visible: true
                }
                PropertyChanges {
                    target: progressBar;
                    value: releases.variant.progress.ratio;
                }
                StateChangeScript {
                    name: "colorChange"
                    script:  {
                        if (progressBar.hasOwnProperty("progressBarColor")) {
                            progressBar.progressBarColor = Qt.lighter("green")
                        }
                    }
                }
            },
            State {
                name: "ready_no_drives"
                when: releases.variant.status === Variant.READY && drives.length <= 0
            },
            State {
                name: "ready"
                when: releases.variant.status === Variant.READY && drives.length > 0
                PropertyChanges {
                    target: messageLoseData;
                    visible: true
                }
                PropertyChanges {
                    target: rightButton;
                    enabled: true;
                    highlighted: true
                    onClicked: drives.selected.write(releases.variant)
                }
                StateChangeScript {
                    name: "colorChange"
                    script: if (rightButton.hasOwnProperty("destructiveAction")) {
                                rightButton.destructiveAction = true
                            }
                }
            },
            State {
                name: "writing_not_possible"
                when: releases.variant.status === Variant.WRITING_NOT_POSSIBLE
                PropertyChanges {
                    target: driveCombo;
                    enabled: false;
                    placeholderText: qsTr("Writing is not possible")
                }
            },
            State {
                name: "writing"
                when: releases.variant.status === Variant.WRITING
                PropertyChanges {
                    target: messageDriveSize
                    enabled: false
                }
                PropertyChanges {
                    target: messageRestore;
                    visible: true
                }
                PropertyChanges {
                    target: driveCombo;
                    enabled: false
                }
                PropertyChanges {
                    target: progressBar;
                    value: drives.selected.progress.ratio;
                }
                StateChangeScript {
                    name: "colorChange"
                    script:  {
                        if (progressBar.hasOwnProperty("destructiveAction")) {
                            progressBar.destructiveAction = false
                        }
                        if (progressBar.hasOwnProperty("progressBarColor")) {
                            progressBar.progressBarColor = "red"
                        }
                    }
                }
            },
            State {
                name: "write_verifying"
                when: releases.variant.status === Variant.WRITE_VERIFYING
                PropertyChanges {
                    target: messageDriveSize
                    enabled: false
                }
                PropertyChanges {
                    target: messageRestore;
                    visible: true
                }
                PropertyChanges {
                    target: driveCombo;
                    enabled: false
                }
                PropertyChanges {
                    target: progressBar;
                    value: drives.selected.progress.ratio;
                }
                StateChangeScript {
                    name: "colorChange"
                    script:  {
                        if (progressBar.hasOwnProperty("progressBarColor")) {
                            progressBar.progressBarColor = Qt.lighter("green")
                        }
                    }
                }
            },
            State {
                name: "finished"
                when: releases.variant.status === Variant.FINISHED
                PropertyChanges {
                    target: messageDriveSize
                    enabled: false
                }
                PropertyChanges {
                    target: messageRestore;
                    visible: true
                }
                PropertyChanges {
                    target: leftButton;
                    text: qsTr("Close");
                    highlighted: true
                    onClicked: {
                        dialog.close()
                    }
                }
                PropertyChanges {
                    target: deleteButton
                    state: "ready"
                }
            },
            State {
                name: "failed_verification_no_drives"
                when: releases.variant.status === Variant.FAILED_VERIFICATION && drives.length <= 0
                PropertyChanges {
                    target: rightButton;
                    text: qsTr("Retry");
                    enabled: false;
                    highlighted: true
                    onClicked: drives.selected.write(releases.variant)
                }
                StateChangeScript {
                    name: "colorChange"
                    script: if (rightButton.hasOwnProperty("destructiveAction")) {
                                rightButton.destructiveAction = true
                            }
                }
            },
            State {
                name: "failed_verification"
                when: releases.variant.status === Variant.FAILED_VERIFICATION && drives.length > 0
                PropertyChanges {
                    target: messageLoseData;
                    visible: true
                }
                PropertyChanges {
                    target: rightButton;
                    text: qsTr("Retry");
                    enabled: true;
                    highlighted: true
                    onClicked: drives.selected.write(releases.variant)
                }
                StateChangeScript {
                    name: "colorChange"
                    script: if (rightButton.hasOwnProperty("destructiveAction")) {
                                rightButton.destructiveAction = true
                            }
                }
            },
            State {
                name: "failed_download"
                when: releases.variant.status === Variant.FAILED_DOWNLOAD
                PropertyChanges {
                    target: driveCombo;
                    enabled: false
                }
                PropertyChanges {
                    target: rightButton;
                    text: qsTr("Retry");
                    enabled: true;
                    highlighted: true
                    onClicked: releases.variant.download()
                }
            },
            State {
                name: "failed_no_drives"
                when: releases.variant.status === Variant.FAILED && drives.length <= 0
                PropertyChanges {
                    target: rightButton;
                    text: qsTr("Retry");
                    enabled: false;
                    highlighted: true
                    onClicked: drives.selected.write(releases.variant)
                }
                StateChangeScript {
                    name: "colorChange"
                    script: if (rightButton.hasOwnProperty("destructiveAction")) {
                                rightButton.destructiveAction = true
                            }
                }
            },
            State {
                name: "failed"
                when: releases.variant.status === Variant.FAILED && drives.length > 0
                PropertyChanges {
                    target: messageLoseData;
                    visible: true
                }
                PropertyChanges {
                    target: rightButton;
                    text: qsTr("Retry");
                    enabled: true;
                    highlighted: true
                    onClicked: drives.selected.write(releases.variant)
                }
                StateChangeScript {
                    name: "colorChange"
                    script: if (rightButton.hasOwnProperty("destructiveAction")) {
                                rightButton.destructiveAction = true
                            }
                }
            }
        ]

        Keys.onEscapePressed: {
            if ([Variant.WRITING, Variant.WRITE_VERIFYING].indexOf(releases.variant.status) < 0)
                dialog.visible = false
        }

        ColumnLayout {
            id: layout
            spacing: units.largeSpacing * 2
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
                leftMargin: 2 * units.gridUnit
                rightMargin: 2 * units.gridUnit
                topMargin: units.gridUnit
            }
            Column {
                id: infoColumn
                spacing: units.smallSpacing

                InfoMessage {
                    id: messageDownload
                    visible: false
                    text: qsTr("The file will be saved to your Downloads folder.")
                    width: layout.width
                }

                InfoMessage {
                    id: messageLoseData
                    visible: false
                    text: qsTr("By writing, you will lose all of the data on %1.").arg(driveCombo.currentText)
                    width: layout.width
                }

                InfoMessage {
                    id: messageRestore
                    visible: false
                    text: qsTr("Your drive will be resized to a smaller capacity. You may resize it back to normal by using Fedora Media Writer; this will remove installation media from your drive.")
                    width: layout.width
                }

                InfoMessage {
                    id: messageSelectedImage
                    visible: releases.selected.isLocal
                    text: "<font color=\"gray\">" + qsTr("Selected:") + "</font> " + (releases.variant.iso ? (((String)(releases.variant.iso)).split("/").slice(-1)[0]) : ("<font color=\"gray\">" + qsTr("None") + "</font>"))
                    width: layout.width
                }

                InfoMessage {
                    id: messageArmBoard
                    visible: boardCombo.otherSelected
                    text: qsTr("Your board or device is not supported by Fedora Media Writer yet. Please check <a href=%1>this page</a> for more information about its compatibility with Fedora and how to create bootable media for it.").arg("https://fedoraproject.org/wiki/Architectures/ARM")
                    width: layout.width
                }

                InfoMessage {
                    id: messageDriveSize
                    enabled: true
                    visible: enabled && drives.selected && drives.selected.size > 160 * 1024 * 1024 * 1024 // warn when it's more than 160GB
                    text: qsTr("The selected drive's size is %1. It's possible you have selected an external drive by accident!").arg(drives.selected ? drives.selected.readableSize : "N/A")
                    width: layout.width
                }

                InfoMessage {
                    error: true
                    visible: releases.variant && releases.variant.errorString.length > 0
                    text: releases.variant ? releases.variant.errorString : ""
                    width: layout.width
                }
            }

            ColumnLayout {
                width: parent.width
                spacing: units.smallSpacing

                Behavior on y {
                    NumberAnimation {
                        duration: 1000
                    }
                }

                QQC2.Label {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    horizontalAlignment: Text.AlignHCenter
                    property double leftSize: releases.variant.progress.to - releases.variant.progress.value
                    property string leftStr:  leftSize <= 0                    ? "" :
                                                                                 (leftSize < 1024)                 ? qsTr("(%1 B left)").arg(leftSize) :
                                                                                                                     (leftSize < (1024 * 1024))        ? qsTr("(%1 KB left)").arg((leftSize / 1024).toFixed(1)) :
                                                                                                                                                         (leftSize < (1024 * 1024 * 1024)) ? qsTr("(%1 MB left)").arg((leftSize / 1024 / 1024).toFixed(1)) :
                                                                                                                                                                                             qsTr("(%1 GB left)").arg((leftSize / 1024 / 1024 / 1024).toFixed(1))
                    text: releases.variant.statusString + (releases.variant.status == Variant.DOWNLOADING ? (" " + leftStr) : "")
                }
                QQC2.ProgressBar {
                    id: progressBar
                    Layout.fillWidth: true
                    value: 0.0
                }
                QQC2.CheckBox {
                    id: writeImmediately
                    enabled: driveCombo.count && opacity > 0.0
                    visible: platformSupportsDelayedWriting
                    opacity: (releases.variant.status == Variant.DOWNLOADING || (releases.variant.status == Variant.DOWNLOAD_VERIFYING && releases.variant.progress.ratio < 0.95)) ? 1.0 : 0.0
                    text: qsTr("Write the image immediately when the download is finished")
                    Binding on checked {
                        value: drives.selected ? drives.selected.delayedWrite : false
                    }
                    Binding {
                        target: drives.selected
                        property: "delayedWrite"
                        value: writeImmediately.checked
                    }
                    onCheckedChanged: {
                        if (checked) {
                            if (drives.selected)
                                drives.selected.setImage(releases.variant)
                        }
                        else {
                            if (drives.selected)
                                drives.selected.setImage(false)
                        }
                    }
                }
            }

            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: units.gridUnit * 2
                Image {
                    source: releases.selected.icon
                    Layout.preferredWidth: Math.round(units.gridUnit * 3.5)
                    Layout.preferredHeight: Math.round(units.gridUnit * 3.5)
                    sourceSize.width: Math.round(units.gridUnit * 3.5)
                    sourceSize.height: Math.round(units.gridUnit * 3.5)
                    fillMode: Image.PreserveAspectFit
                }
                Arrow {
                    id: writeArrow
                    Layout.alignment: Qt.AlignVCenter
                    scale: 1.4
                    SequentialAnimation {
                        running: releases.variant.status == Variant.WRITING
                        loops: -1
                        onStopped: {
                            if (releases.variant.status == Variant.FINISHED)
                                writeArrow.color = "#00dd00"
                            else
                                writeArrow.color = palette.text
                        }
                        ColorAnimation {
                            duration: 3500
                            target: writeArrow
                            property: "color"
                            to: "red"
                        }
                        PauseAnimation {
                            duration: 500
                        }
                        ColorAnimation {
                            duration: 3500
                            target: writeArrow
                            property: "color"
                            to: palette.text
                        }
                        PauseAnimation {
                            duration: 500
                        }
                    }
                }
                ColumnLayout {
                    spacing: units.largeSpacing

                    QQC2.ComboBox {
                        id: driveCombo
                        z: pressed ? 1 : 0
                        model: drives
                        enabled: !(currentIndex === -1 || !currentText)
                        displayText: currentIndex === -1 || !currentText ? qsTr("There are no portable drives connected") : currentText
                        textRole: "display"

                        Binding on currentIndex {
                            when: drives
                            value: drives.selectedIndex
                        }
                        Binding {
                            target: drives
                            property: "selectedIndex"
                            value: driveCombo.currentIndex
                        }
                        onActivated: {
                            if ([Variant.FINISHED, Variant.FAILED, Variant.FAILED_VERIFICATION].indexOf(releases.variant.status) >= 0)
                                releases.variant.resetStatus()
                        }
                    }
                    QQC2.ComboBox {
                        id: boardCombo
                        z: pressed ? 1 : 0
                        enabled: visible
                        visible: releases.selected.version.variant.arch.id == Architecture.ARM || (releases.selected.isLocal && releases.variant.iso.indexOf(".iso", releases.variant.iso.length - ".iso".length) === -1)
                        property bool otherSelected: currentIndex === (count - 1)
                        model: ["Raspberry Pi 2 Model B", "Raspberry Pi 3 Model B", qsTr("Other")]
                    }
                }
            }

            ColumnLayout {
                z: -1
                Layout.maximumWidth: parent.width
                spacing: units.smallSpacing
                Item {
                    height: units.smallSpacing
                    width: 1
                }
                RowLayout {
                    height: rightButton.height
                    Layout.minimumWidth: parent.width
                    Layout.maximumWidth: parent.width
                    spacing: units.largeSpacing

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }

                    DeleteButton {
                        id: deleteButton
                        Layout.fillHeight: true
                        Layout.maximumWidth: parent.width - leftButton.width - rightButton.width - parent.spacing * 2
                        state: "hidden"
                        errorText: qsTr("It was not possible to delete<br>\"<a href=\"%1\">%2</a>\".").arg(releases.variant.iso.match(".*/")).arg(releases.variant.iso)
                        onStarted: {
                            if (releases.variant.erase())
                                state = "success"
                            else
                                state = "error"
                        }
                    }
                    QQC2.Button {
                        id: leftButton
                        Layout.alignment: Qt.AlignRight
                        Behavior on implicitWidth { NumberAnimation { duration: 80 } }
                        text: qsTr("Cancel")
                        enabled: true
                        onClicked: {
                            if (drives.selected)
                                drives.selected.cancel()
                            releases.variant.resetStatus()
                            dialog.close()
                        }
                    }
                    QQC2.Button {
                        id: rightButton
                        Layout.alignment: Qt.AlignRight
                        Behavior on implicitWidth { NumberAnimation { duration: 80 } }
                        text: qsTr("Write to Disk")
                        enabled: false
                    }
                }
            }
        }
    }
}
