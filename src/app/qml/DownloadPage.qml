/*
 * Fedora Media Writer
 * Copyright (C) 2021-2022 Evžen Gasta <evzen.ml@seznam.cz>
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

import QtQuick 6.2
import QtQuick.Controls 6.2
import QtQuick.Window 6.2
import QtQuick.Layouts 6.2
import QtQml 6.2


Page {
    id: downloadPage

    property int availableDrives: drives.length
    property int currentStatus: releases.variant.status

    ColumnLayout {
        id: mainColumn
        anchors.fill: parent
        spacing: units.gridUnit
        
        Image {
            source: "qrc:/downloadPageImage"
            Layout.fillHeight: true
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectFit
            sourceSize.width: parent.width
            sourceSize.height: parent.height
            smooth: true
            antialiasing: true
        }

        Heading {
            id: heading
            property string file: mainWindow.selectedOption == Units.MainSelect.Write ? (String)(releases.localFile.iso).split("/").slice(-1)[0] : releases.selected.name + " " + releases.selected.version.number
            
            Layout.alignment: Qt.AlignHCenter
            text: {
                if (currentStatus === Units.DownloadStatus.Finished)
                    qsTr("%1 Successfully Written").arg(file)
                else if (currentStatus === Units.DownloadStatus.Writing)
                    qsTr("Writing %1").arg(file)
                else if (currentStatus === Units.DownloadStatus.Downloading)
                    qsTr("Downloading %1").arg(file)
                else if (currentStatus === Units.DownloadStatus.Preparing)
                    qsTr("Preparing %1").arg(file)
                else if (currentStatus === Units.DownloadStatus.Ready)
                    qsTr("Ready to write %1").arg(file)
                else if (currentStatus == Units.DownloadStatus.Failed_Download)
                    qsTr("Failed to download %1").arg(file)
                else
                    releases.variant.statusString
            }
            level: 4
            Layout.preferredWidth: mainColumn.width
            elide: Label.ElideRight
            horizontalAlignment: Label.AlignHCenter
        }
        
        ColumnLayout {
            id: progressColumn
            property double leftSize: releases.variant.progress.to - releases.variant.progress.value
            property string rightStr: leftSize <= 0 ? "" :
                                                      (leftSize < 1024) ? qsTr(" B left)") :
                                                                          (leftSize < (1024 * 1024)) ? qsTr(" KB left)") :
                                                                                                       (leftSize < (1024 * 1024 * 1024)) ? qsTr(" MB left)") :
                                                                                                                                           qsTr(" GB left)")

            property string leftStr: leftSize <= 0 ? "" :
                                                     (leftSize < 1024) ? qsTr(" (%1").arg(leftSize) :
                                                                         (leftSize < (1024 * 1024)) ? qsTr(" (%1").arg((leftSize / 1024).toFixed(1)) :
                                                                                                      (leftSize < (1024 * 1024 * 1024)) ? qsTr(" (%1").arg((leftSize / 1024 / 1024).toFixed(1)) :
                                                                                                                                          qsTr(" (%1").arg((leftSize / 1024 / 1024 / 1024).toFixed(1))

            TextMetrics {
                id: fontMetrics
                text: progressColumn.leftStr.replace(/[1-9]/g, '0')
            }

            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 0

                Label {
                    id: refLabel
                    text: {
                        if ((currentStatus == Units.DownloadStatus.Failed_Verification ||
                             currentStatus == Units.DownloadStatus.Failed) && !availableDrives)
                            qsTr("Your drive was unplugged during the process")
                        else
                            releases.variant.statusString
                    }
                    visible: text !== heading.text
                }

                Label {
                    visible: currentStatus == Units.DownloadStatus.Downloading
                    Layout.preferredWidth: fontMetrics.width
                    text: progressColumn.leftStr
                }

                Label {
                    visible: currentStatus == Units.DownloadStatus.Downloading
                    text: progressColumn.rightStr
                }
            }

            ProgressBar {
                id: progressBar
                Layout.topMargin: units.gridUnit / 2
                Layout.fillWidth: true
                value: 0.0
            }
        }
        
        Column {
            id: infoColumn
            spacing: units.gridUnit / 2
            
            Label {
                id: messageDownload
                visible: false
                text: qsTr("Downloads are saved to the downloads folder.")
                wrapMode: Label.Wrap
                width: mainColumn.width
            }

            Label {
                id: messageLoseData
                visible: false
                text: qsTr("By writing, you will lose all of the data on %1.").arg(drives.selected.name)
                wrapMode: Label.Wrap
                width: mainColumn.width
            }

            Label {
                id: messageInsertDrive
                visible: false
                text: qsTr("Please insert an USB drive.")
                wrapMode: Label.Wrap
                width: mainColumn.width
            }

            Label {
                id: messageRestore
                visible: false
                text: qsTr("Your drive will be resized to a smaller capacity. You may resize it back to normal by using Fedora Media Writer. This will remove installation media from your drive.")
                wrapMode: Label.Wrap
                width: mainColumn.width
            }

            Label {
                id: messageSelectedImage
                visible: releases.selected.isLocal
                text: "<font color=\"gray\">" + qsTr("Selected:") + "</font> " + (releases.variant.iso ? (((String)(releases.variant.iso)).split("/").slice(-1)[0]) : ("<font color=\"gray\">" + qsTr("None") + "</font>"))
                wrapMode: Label.Wrap
                width: mainColumn.width
            }

            Label {
                id: messageArmBoard
                visible: false //boardCombo.otherSelected
                text: qsTr("Your board or device is not supported by Fedora Media Writer yet. Please check <a href=%1>this page</a> for more information about its compatibility with Fedora and how to create bootable media for it.").arg("https://fedoraproject.org/wiki/Architectures/ARM")
                wrapMode: Label.Wrap
                width: mainColumn.width
            }

            Label {
                id: messageDriveSize
                enabled: true
                visible: enabled && drives.selected && drives.selected.size > 160 * 1024 * 1024 * 1024 // warn when it's more than 160GB
                text: qsTr("The selected drive's size is %1. It's possible you have selected an external drive by accident!").arg(drives.selected ? drives.selected.readableSize : "N/A")
                wrapMode: Label.Wrap
                width: mainColumn.width
            }
            
            Label {
                id: messageFinished
                enabled: true
                visible: false
                text: qsTr("Restart and boot from %1 to start using %2.").arg(drives.selected ? drives.selected.name : "N/A").arg(mainWindow.selectedOption == Units.MainSelect.Write ? (String)(releases.localFile.iso).split("/").slice(-1)[0] : releases.selected.name)
                wrapMode: Label.Wrap
                width: mainColumn.width
            }

            Label {
                visible: releases.variant && releases.variant.errorString.length > 0
                text: releases.variant ? releases.variant.errorString : ""
                wrapMode: Label.Wrap
                width: mainColumn.width
            }
        }
    }
    
    states: [
        State {
            name: "downloading"
            when: currentStatus === Units.DownloadStatus.Downloading
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
            when: currentStatus === Units.DownloadStatus.Download_Verifying
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
            name: "ready_no_drives"
            when: currentStatus === Units.DownloadStatus.Ready && !availableDrives
            PropertyChanges {
                target: messageInsertDrive
                visible: true
            }
        },
        State {
            name: "ready"
            when: currentStatus === Units.DownloadStatus.Ready && availableDrives
            PropertyChanges {
                target: messageLoseData;
                visible: true
            }
        },
        State {
            name: "writing"
            when: currentStatus === Units.DownloadStatus.Writing
            PropertyChanges {
                target: messageDriveSize
                enabled: false
            }
            PropertyChanges {
                target: messageRestore;
                visible: true
            }
            PropertyChanges {
                target: progressBar;
                value: drives.selected.progress.ratio;
            }
        },
        State {
            name: "write_verifying"
            when: currentStatus === Units.DownloadStatus.Write_Verifying
            PropertyChanges {
                target: messageDriveSize
                enabled: false
            }
            PropertyChanges {
                target: messageRestore;
                visible: true
            }
            PropertyChanges {
                target: progressBar;
                value: drives.selected.progress.ratio;
            }
        },
        State {
            name: "finished"
            when: currentStatus === Units.DownloadStatus.Finished
            PropertyChanges {
                target: messageDriveSize;
                enabled: false
            }
            PropertyChanges {
                target: messageFinished;
                visible: true
            }
            PropertyChanges {
                target: progressBar;
                value: 100;
            }
            StateChangeScript {
                script: { 
                    if (cancelDialog.visible)
                        cancelDialog.close()
                    if (mainWindow.eraseVariant)
                        releases.variant.erase()
                    else
                        releases.variant
                }   
            }
        },
        State {
            name: "failed_verification"
            when: currentStatus === Units.DownloadStatus.Failed_Verification && availableDrives
            PropertyChanges {
                target: messageLoseData;
                visible: true
            }
        },
        State {
            name: "failed"
            when: currentStatus === Units.DownloadStatus.Failed && availableDrives
            PropertyChanges {
                target: messageLoseData;
                visible: true
            }
        }
        // Unhandled states:
        // preparing. writing_not_possible, failed_verification_no_drives,failed_download, failed_no_drives
    ]    

    onCurrentStatusChanged: {
        // Not sure it's necessary, but it doesn't hurt to be safe
        if (mainWindow.selectedPage == Units.Page.DownloadPage) {
            prevButton.visible = getPrevButtonState()
            nextButton.visible = getNextButtonState()
        }
    }

    onAvailableDrivesChanged: {
        // Not sure it's necessary, but it doesn't hurt to be safe
        if (mainWindow.selectedPage == Units.Page.DownloadPage) {
            nextButton.visible = getNextButtonState()
        }
    }

    function getPrevButtonState() {
        // There will be only [Finish] button on the right side so [Cancel] button
        // is not necessary
        return currentStatus != Units.DownloadStatus.Finished
    }

    function getNextButtonState() {
        // This will be [Finish] button to successfully go to the main page
        if (currentStatus == Units.DownloadStatus.Finished)
            return true
        // This will be [Retry] button to start the process again if there is a drive plugged in
        else if (currentStatus == Units.DownloadStatus.Ready ||
            currentStatus == Units.DownloadStatus.Failed_Verification ||
            currentStatus == Units.DownloadStatus.Failed) {
            return availableDrives
        }

        return false
    }
}

