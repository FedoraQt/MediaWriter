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

/*
    Connections {
        id: downloadWait
        target: liveUSBData.currentImage
        onReadyToWriteChanged: {
            if (liveUSBData.currentImage.readyToWrite && writeImmediately.checked) {
                liveUSBData.currentImage.write()
            }
        }
    }

    Connections {
        target: liveUSBData.currentImage.writer
        onFinishedChanged: {
            writeImmediately.checked = false
        }
    }
*/
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
                        /*
                        RowLayout {
                            width: infoColumn.width
                            spacing: $(8)

                            Rectangle {
                                Layout.fillWidth: false
                                Layout.alignment: Qt.AlignVCenter
                                width: $(18)
                                height: $(18)
                                radius: width / 2
                                color: "#729FCF"
                                border {
                                    width: $(1)
                                    color: "#a1a1a1"
                                }
                                Text {
                                    anchors.centerIn: parent
                                    verticalAlignment: Text.AlignVCenter
                                    color: "white"
                                    text: "!"
                                    rotation: 180
                                    font.bold: true
                                    font.pixelSize: $(16)
                                }
                            }
                            Text {
                                Layout.fillHeight: true
                                Layout.fillWidth: true
                                verticalAlignment: Text.AlignVCenter
                                wrapMode: Text.Wrap
                                font.pixelSize: $(12)
                                text: qsTr("PLACEHOLDER")
                            }
                        }
                        */
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
                            property string leftStr: leftSize <= 0 ? "" :
                                                     (leftSize < 1024) ? (leftSize + " B") :
                                                     (leftSize < (1024 * 1024)) ? ((leftSize / 1024).toFixed(1) + " KB") :
                                                     (leftSize < (1024 * 1024 * 1024)) ? ((leftSize / 1024 / 1024).toFixed(1) + " MB") :
                                                     ((leftSize / 1024 / 1024 / 1024).toFixed(1) + " GB")
                            text: releases.selected.version.variant.statusString + ((releases.selected.version.variant.status == Variant.DOWNLOADING && leftStr.length > 0) ? " (" + qsTr("%1 left").arg(leftStr) + ")" : "")
                        }
                        Item {
                            Layout.fillWidth: true
                            height: childrenRect.height
                            AdwaitaProgressBar {
                                width: parent.width
                                progressColor: releases.selected.version.variant.status == Variant.WRITING ? "red" :
                                               releases.selected.version.variant.status == Variant.DOWNLOAD_VERIFYING ? Qt.lighter("green") : "#54aada"
                                value: releases.selected.version.variant.status == Variant.DOWNLOADING ? releases.selected.version.variant.progress.ratio :
                                       releases.selected.version.variant.status == Variant.WRITING ? drives.selected.progress.ratio :
                                       releases.selected.version.variant.status == Variant.DOWNLOAD_VERIFYING ? releases.selected.version.variant.progress.ratio : 0.0
                            }
                        }
                        AdwaitaCheckBox {
                            id: writeImmediately
                            enabled: driveCombo.count && opacity > 0.0
                            //opacity: !liveUSBData.currentImage.readyToWrite && liveUSBData.currentImage.download.running && liveUSBData.currentImage.download.progress / liveUSBData.currentImage.download.maxProgress < 0.95 ? 1.0 : 0.0
                            opacity: (releases.selected.version.variant.status == Variant.DOWNLOADING && releases.selected.version.variant.ratio < 0.95) ? 1.0 : 0.0
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
                                //enabled: !liveUSBData.currentImage.writer.running && !liveUSBData.currentImage.writer.finished
                                onClicked: {
                                    //liveUSBData.currentImage.download.cancel()
                                    //liveUSBData.currentImage.writer.cancel()
                                    //liveUSBData.currentImage.writer.finished = false
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
                                       releases.selected.version.variant.status == Variant.FAILED_DOWNLOAD ? "#628fcf" : "red"
                                textColor: enabled ? "white" : palette.text
                                enabled: ((releases.selected.version.variant.status == Variant.READY ||
                                          releases.selected.version.variant.status == Variant.FINISHED ||
                                          releases.selected.version.variant.status == Variant.FAILED) && drives.length > 0)
                                         || releases.selected.version.variant.status == Variant.FAILED_DOWNLOAD
                                text: releases.selected.version.variant.status == Variant.FINISHED          ? qsTr("Close") :
                                      (releases.selected.version.variant.status == Variant.FAILED_DOWNLOAD ||
                                       releases.selected.version.variant.status == Variant.FAILED         ) ? qsTr("Retry") :
                                                                                                              qsTr("Write to disk")
                                onClicked: {
                                    if (releases.selected.version.variant.status == Variant.READY || releases.selected.version.variant.status == Variant.FAILED) {
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
