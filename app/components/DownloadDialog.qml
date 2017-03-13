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
        writeArrow.color = palette.text
        writeImmediately.checked = false
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
        onSelectedChanged: {
            reset();
        }
    }

    Connections {
        target: drives
        onSelectedChanged: {
            writeImmediately.checked = false
        }
    }

    Connections {
        target: releases.variant
        onStatusChanged: {
            if (releases.variant.status == Variant.FINISHED || releases.variant.status == Variant.FAILED || releases.variant.status == Variant.FAILED_DOWNLOAD)
                writeImmediately.checked = false
        }
    }

    contentItem: Rectangle {
        id: contentWrapper
        anchors.fill: parent
        color: palette.window
        ScrollView {
            id: contentScrollView
            anchors.fill: parent
            contentItem: Item {
                width: contentScrollView.width
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

                        InfoMessage {
                            width: infoColumn.width
                            visible: drives.length > 0 &&
                                     (releases.variant.status == Variant.READY ||
                                      releases.variant.status == Variant.FAILED ||
                                      releases.variant.status == Variant.FAILED_VERIFICATION)
                            text: qsTr("By writing, you will lose all of the data on %1.").arg(driveCombo.currentText)
                        }

                        InfoMessage {
                            width: infoColumn.width
                            visible: releases.variant.status == Variant.WRITING ||
                                     releases.variant.status == Variant.WRITE_VERIFYING ||
                                     releases.variant.status == Variant.FINISHED
                            text: qsTr("Your computer will now report this drive is much smaller than it really is. Just insert your drive again while Fedora Media Writer is running and you'll be able to restore it back to its full size.")
                        }

                        InfoMessage {
                            width: infoColumn.width
                            visible: releases.selected.isLocal
                            text: "<font color=\"gray\">" + qsTr("Selected:") + "</font> " + (releases.variant.iso ? (((String)(releases.variant.iso)).split("/").slice(-1)[0]) : ("<font color=\"gray\">" + qsTr("None") + "</font>"))
                        }

                        InfoMessage {
                            error: true
                            width: infoColumn.width
                            visible: releases.variant && releases.variant.errorString.length > 0
                            text: releases.variant ? releases.variant.errorString : ""
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
                            property double leftSize: releases.variant.progress.to - releases.variant.progress.value
                            property string leftStr:  leftSize <= 0                    ? "" :
                                                     (leftSize < 1024)                 ? qsTr("(%1 B left)").arg(leftSize) :
                                                     (leftSize < (1024 * 1024))        ? qsTr("(%1 KB left)").arg((leftSize / 1024).toFixed(1)) :
                                                     (leftSize < (1024 * 1024 * 1024)) ? qsTr("(%1 MB left)").arg((leftSize / 1024 / 1024).toFixed(1)) :
                                                                                         qsTr("(%1 GB left)").arg((leftSize / 1024 / 1024 / 1024).toFixed(1))
                            text: releases.variant.statusString + (releases.variant.status == Variant.DOWNLOADING ? (" " + leftStr) : "")
                            color: palette.windowText
                        }
                        Item {
                            Layout.fillWidth: true
                            height: childrenRect.height
                            AdwaitaProgressBar {
                                width: parent.width
                                progressColor: releases.variant.status == Variant.WRITING            ? "red" :
                                               releases.variant.status == Variant.DOWNLOAD_VERIFYING ? Qt.lighter("green") :
                                               releases.variant.status == Variant.WRITE_VERIFYING    ? Qt.lighter("green") :
                                                                                                                        "#54aada"
                                value: releases.variant.status == Variant.DOWNLOADING ? releases.variant.progress.ratio :
                                       releases.variant.status == Variant.WRITING ? drives.selected.progress.ratio :
                                       releases.variant.status == Variant.DOWNLOAD_VERIFYING ? releases.variant.progress.ratio :
                                       releases.variant.status == Variant.WRITE_VERIFYING ? drives.selected.progress.ratio : 0.0
                            }
                        }
                        AdwaitaCheckBox {
                            id: writeImmediately
                            enabled: driveCombo.count && opacity > 0.0
                            opacity: (releases.variant.status == Variant.DOWNLOADING || (releases.variant.status == Variant.DOWNLOAD_VERIFYING && releases.variant.progress.ratio < 0.95)) ? 1.0 : 0.0
                            text: qsTr("Write the image immediately when the download is finished")
                            onCheckedChanged: {
                                if (drives.selected) {
                                    drives.selected.cancel()
                                    if (checked)
                                        drives.selected.write(releases.variant)
                                }
                            }
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
                        AdwaitaComboBox {
                            id: driveCombo
                            Layout.preferredWidth: implicitWidth * 2.5
                            model: drives
                            textRole: "display"
                            currentIndex: drives.selectedIndex
                            onCurrentIndexChanged: {
                                drives.selectedIndex = currentIndex
                                releases.variant.resetStatus()
                            }
                            onModelChanged: {
                                if (drives.length <= 0)
                                    currentIndex = -1
                                currentIndex = drives.selectedIndex
                            }
                            enabled: releases.variant.status != Variant.WRITING &&
                                     releases.variant.status != Variant.WRITE_VERIFYING &&
                                     releases.variant.status != Variant.FAILED_DOWNLOAD &&
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
                                enabled: releases.variant.status != Variant.FINISHED
                                onClicked: {
                                    releases.variant.resetStatus()
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
                                color: releases.variant.status == Variant.FINISHED ||
                                       releases.variant.status == Variant.FAILED_DOWNLOAD ? "#628fcf" :
                                       releases.variant.status == Variant.FAILED_VERIFICATION ? "#628fcf" : "red"
                                textColor: enabled ? "white" : palette.text
                                enabled: ((releases.variant.status == Variant.READY ||
                                          releases.variant.status == Variant.FAILED) && drives.length > 0)
                                         || releases.variant.status == Variant.FINISHED
                                         || releases.variant.status == Variant.FAILED_DOWNLOAD
                                         || releases.variant.status == Variant.FAILED_VERIFICATION
                                text: releases.variant.status == Variant.FINISHED                ? qsTr("Close") :
                                      (releases.variant.status == Variant.FAILED_DOWNLOAD ||
                                       releases.variant.status == Variant.FAILED_VERIFICATION ||
                                       releases.variant.status == Variant.FAILED               ) ? qsTr("Retry") :
                                                                                                                    qsTr("Write to disk")
                                onClicked: {
                                    if (releases.variant.status == Variant.READY || releases.variant.status == Variant.FAILED || releases.variant.status == Variant.FAILED_VERIFICATION) {
                                        drives.selected.write(releases.variant)
                                    }
                                    else if (releases.variant.status == Variant.FINISHED) {
                                        releases.variant.resetStatus()
                                        writeImmediately.checked = false
                                        root.close()
                                    }
                                    else if (releases.variant.status == Variant.FAILED_DOWNLOAD) {
                                        releases.variant.download()
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
