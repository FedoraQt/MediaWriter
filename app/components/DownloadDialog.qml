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

import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1
import QtQuick.Window 2.0

import MediaWriter 1.0

Dialog {
    id: root
    title: qsTr("Write %1").arg(releases.selected.name)

    height: layout.height + $(56)
    standardButtons: StandardButton.NoButton

    width: $(640)

    function reset() {
        writeArrow.color = "black"
        writeImmediately.checked = false
    }

    onVisibleChanged: {
        if (!visible) {
            if (drives.selected)
                drives.selected.cancel()
            releases.selected.version.variant.resetStatus()
            downloadManager.cancel()
        }
        reset()
    }

    Connections {
        target: releases
        onSelectedChanged: {
            reset();
        }
    }

    Connections {
        id: downloadWait
        target: releases.selected.version.variant
        onStatusChanged: {
            if (releases.selected.version.variant.status == Variant.READY && writeImmediately.checked && drives.selected != null) {
                drives.selected.write(releases.selected.version.variant)
            }
        }
    }

    Connections {
        target: drives
        onSelectedChanged: {
            writeImmediately.checked = false
        }
    }

    Connections {
        target: releases.selected.version.variant
        onStatusChanged: {
            if (releases.selected.version.variant.status == Variant.FINISHED || releases.selected.version.variant.status == Variant.FAILED)
                writeImmediately.checked = false
        }
    }

    contentItem: Rectangle {
        id: contentWrapper
        anchors.fill: parent
        color: palette.window
        ScrollView {
            anchors.fill: parent
            horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
            contentItem: Item {
                width: contentWrapper.width
                height: layout.height + $(32)
                Column {
                    id: layout
                    spacing: $(24)
                    anchors {
                        top: parent.top
                        left: parent.left
                        right: parent.right
                        topMargin: $(32)
                        leftMargin: $(48)
                        rightMargin: anchors.leftMargin
                    }
                    Column {
                        id: infoColumn
                        spacing: $(4)
                        width: parent.width

                        RowLayout {
                            visible: drives.length > 0 &&
                                     (releases.selected.version.variant.status == Variant.READY ||
                                      releases.selected.version.variant.status == Variant.FAILED ||
                                      releases.selected.version.variant.status == Variant.FAILED_VERIFICATION)
                            width: infoColumn.width
                            spacing: $(8)
                            Rectangle {
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
                                    font.pixelSize: $(12)
                                    color: "white"
                                }
                            }
                            Text {
                                Layout.fillHeight: true
                                Layout.fillWidth: true
                                verticalAlignment: Text.AlignVCenter
                                wrapMode: Text.Wrap
                                font.pixelSize: $(12)
                                text: qsTr("By writing, you will lose all of the data on %1.").arg(driveCombo.currentText)
                            }
                        }

                        RowLayout {
                            visible: releases.selected.version.variant.status == Variant.WRITING ||
                                     releases.selected.version.variant.status == Variant.WRITE_VERIFYING ||
                                     releases.selected.version.variant.status == Variant.FINISHED
                            width: infoColumn.width
                            spacing: $(8)
                            Rectangle {
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
                                    font.pixelSize: $(12)
                                    color: "white"
                                }
                            }
                            Text {
                                Layout.fillHeight: true
                                Layout.fillWidth: true
                                verticalAlignment: Text.AlignVCenter
                                wrapMode: Text.Wrap
                                font.pixelSize: $(12)
                                text: qsTr("Your computer will now report this drive is much smaller than it really is. Just insert your drive again while Fedora Media Writer is running and you'll be able to restore it back to its full size.")
                            }
                        }

                        RowLayout {
                            visible: releases.selected.version.variant && releases.selected.version.variant.errorString.length > 0
                            width: infoColumn.width
                            spacing: $(8)
                            Rectangle {
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
                                Layout.fillHeight: true
                                Layout.fillWidth: true
                                verticalAlignment: Text.AlignVCenter
                                wrapMode: Text.Wrap
                                font.pixelSize: $(12)
                                text: releases.selected.version.variant ? releases.selected.version.variant.errorString : ""
                            }
                        }
                        RowLayout {
                            visible: releases.selected.isLocal
                            width: infoColumn.width
                            spacing: $(12)
                            Text {
                                Layout.fillWidth: false
                                Layout.alignment: Qt.AlignVCenter
                                verticalAlignment: Text.AlignVCenter
                                color: "white"
                                text: "!"
                                rotation: 180
                                font.bold: true
                                font.pixelSize: $(13)

                                Rectangle {
                                    z: -1
                                    anchors.centerIn: parent
                                    height: parent.height
                                    width: height
                                    radius: width / 2
                                    color: "#729FCF"
                                    border {
                                        width: 1
                                        color: "#a1a1a1"
                                    }
                                }
                            }
                            Text {
                                Layout.fillHeight: true
                                Layout.fillWidth: true
                                verticalAlignment: Text.AlignVCenter
                                wrapMode: Text.Wrap
                                font.pixelSize: $(12)
                                text: "<font color=\"gray\">" + qsTr("Selected:") + "</font> " + (releases.selected.version.variant.iso ? (((String)(releases.selected.version.variant.iso)).split("/").slice(-1)[0]) : ("<font color=\"gray\">" + qsTr("None") + "</font>"))
                            }
                        }
                    }

                    ColumnLayout {
                        width: parent.width
                        spacing: $(5)

                        Behavior on y {
                            NumberAnimation {
                                duration: 1000
                            }
                        }

                        Text {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            horizontalAlignment: Text.AlignHCenter
                            font.pixelSize: $(12)
                            property double leftSize: releases.selected.version.variant.progress.to - releases.selected.version.variant.progress.value
                            property string leftStr:  leftSize <= 0                    ? "" :
                                                     (leftSize < 1024)                 ? qsTr("(%1 B left)").arg(leftSize) :
                                                     (leftSize < (1024 * 1024))        ? qsTr("(%1 KB left)").arg((leftSize / 1024).toFixed(1)) :
                                                     (leftSize < (1024 * 1024 * 1024)) ? qsTr("(%1 MB left)").arg((leftSize / 1024 / 1024).toFixed(1)) :
                                                                                         qsTr("(%1 GB left)").arg((leftSize / 1024 / 1024 / 1024).toFixed(1))
                            text: releases.selected.version.variant.statusString + (releases.selected.version.variant.status == Variant.DOWNLOADING ? (" " + leftStr) : "")
                        }
                        Item {
                            Layout.fillWidth: true
                            height: childrenRect.height
                            AdwaitaProgressBar {
                                width: parent.width
                                progressColor: releases.selected.version.variant.status == Variant.WRITING            ? "red" :
                                               releases.selected.version.variant.status == Variant.DOWNLOAD_VERIFYING ? Qt.lighter("green") :
                                               releases.selected.version.variant.status == Variant.WRITE_VERIFYING    ? Qt.lighter("green") :
                                                                                                                        "#54aada"
                                value: releases.selected.version.variant.status == Variant.DOWNLOADING ? releases.selected.version.variant.progress.ratio :
                                       releases.selected.version.variant.status == Variant.WRITING ? drives.selected.progress.ratio :
                                       releases.selected.version.variant.status == Variant.DOWNLOAD_VERIFYING ? releases.selected.version.variant.progress.ratio :
                                       releases.selected.version.variant.status == Variant.WRITE_VERIFYING ? drives.selected.progress.ratio : 0.0
                            }
                        }
                        AdwaitaCheckBox {
                            id: writeImmediately
                            enabled: driveCombo.count && opacity > 0.0
                            opacity: (releases.selected.version.variant.status == Variant.DOWNLOADING || (releases.selected.version.variant.status == Variant.DOWNLOAD_VERIFYING && releases.selected.version.variant.progress.ratio < 0.95)) ? 1.0 : 0.0
                            text: qsTr("Write the image immediately when the download is finished")
                        }
                    }

                    RowLayout {
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: $(32)
                        Image {
                            source: releases.selected.icon
                            Layout.preferredWidth: $(64)
                            Layout.preferredHeight: $(64)
                            sourceSize.width: $(64)
                            sourceSize.height: $(64)
                            fillMode: Image.PreserveAspectFit
                        }
                        Arrow {
                            id: writeArrow
                            anchors.verticalCenter: parent.verticalCenter
                            scale: $(1.4)
                            SequentialAnimation {
                                running: releases.selected.version.variant.status == Variant.WRITING
                                loops: -1
                                onStopped: {
                                    if (releases.selected.version.variant.status == Variant.FINISHED)
                                        writeArrow.color = "#00dd00"
                                    else
                                        writeArrow.color = "black"
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
                                    to: "black"
                                }
                                PauseAnimation {
                                    duration: 500
                                }
                            }
                        }
                        AdwaitaComboBox {
                            id: driveCombo
                            Layout.preferredWidth: implicitWidth * 2.5
                            model: drives
                            textRole: "display"
                            currentIndex: drives.selectedIndex
                            onCurrentIndexChanged: {
                                drives.selectedIndex = currentIndex
                                releases.selected.version.variant.resetStatus()
                            }
                            onModelChanged: {
                                if (drives.length <= 0)
                                    currentIndex = -1
                                currentIndex = drives.selectedIndex
                            }
                            enabled: releases.selected.version.variant.status != Variant.WRITING &&
                                     releases.selected.version.variant.status != Variant.WRITE_VERIFYING &&
                                     releases.selected.version.variant.status != Variant.FAILED_DOWNLOAD &&
                                     drives.length > 0
                            Row {
                                spacing: $(6)
                                anchors.fill: parent
                                anchors.leftMargin: $(12)
                                visible: drives.length <= 0
                                Text {
                                    height: parent.height
                                    verticalAlignment: Text.AlignVCenter
                                    text: qsTr("There are no portable drives connected")
                                    color: "gray"
                                    font.pixelSize: $(12)
                                }
                            }
                        }
                    }

                    ColumnLayout {
                        width: parent.width
                        spacing: $(12)
                        RowLayout {
                            height: acceptButton.height
                            width: parent.width
                            spacing: $(10)

                            Item {
                                Layout.fillWidth: true
                                height: $(1)
                            }

                            AdwaitaButton {
                                id: cancelButton
                                anchors {
                                    right: acceptButton.left
                                    top: parent.top
                                    bottom: parent.bottom
                                    rightMargin: $(6)
                                }
                                text: qsTr("Cancel")
                                enabled: releases.selected.version.variant.status != Variant.FINISHED
                                onClicked: {
                                    releases.selected.version.variant.resetStatus()
                                    writeImmediately.checked = false
                                    root.close()
                                }
                            }
                            AdwaitaButton {
                                id: acceptButton
                                anchors {
                                    right: parent.right
                                    top: parent.top
                                    bottom: parent.bottom
                                }
                                color: releases.selected.version.variant.status == Variant.FINISHED ||
                                       releases.selected.version.variant.status == Variant.FAILED_DOWNLOAD ? "#628fcf" :
                                       releases.selected.version.variant.status == Variant.FAILED_VERIFICATION ? "#628fcf" : "red"
                                textColor: enabled ? "white" : palette.text
                                enabled: ((releases.selected.version.variant.status == Variant.READY ||
                                          releases.selected.version.variant.status == Variant.FAILED) && drives.length > 0)
                                         || releases.selected.version.variant.status == Variant.FINISHED
                                         || releases.selected.version.variant.status == Variant.FAILED_DOWNLOAD
                                         || releases.selected.version.variant.status == Variant.FAILED_VERIFICATION
                                text: releases.selected.version.variant.status == Variant.FINISHED                ? qsTr("Close") :
                                      (releases.selected.version.variant.status == Variant.FAILED_DOWNLOAD ||
                                       releases.selected.version.variant.status == Variant.FAILED_VERIFICATION ||
                                       releases.selected.version.variant.status == Variant.FAILED               ) ? qsTr("Retry") :
                                                                                                                    qsTr("Write to disk")
                                onClicked: {
                                    if (releases.selected.version.variant.status == Variant.READY || releases.selected.version.variant.status == Variant.FAILED || releases.selected.version.variant.status == Variant.FAILED_VERIFICATION) {
                                        drives.selected.write(releases.selected.version.variant)
                                    }
                                    else if (releases.selected.version.variant.status == Variant.FINISHED) {
                                        releases.selected.version.variant.resetStatus()
                                        writeImmediately.checked = false
                                        root.close()
                                    }
                                    else if (releases.selected.version.variant.status == Variant.FAILED_DOWNLOAD) {
                                        releases.selected.version.variant.download()
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
