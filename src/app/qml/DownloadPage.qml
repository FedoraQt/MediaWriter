/*
 * Fedora Media Writer
 * Copyright (C) 2024 Jan Grulich <jgrulich@redhat.com>
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

import QtQuick 6.6
import QtQuick.Controls 6.6 as QQC2
import QtQuick.Layouts 6.6

Page {
    id: downloadPage

    property int availableDrives: drives.length
    property int currentStatus: releases.variant.status
    property string file: mainWindow.selectedOption == Units.MainSelect.Write ? (String)(releases.localFile.iso).split("/").slice(-1)[0] : releases.selected.name + " " + releases.selected.version.number

    imageSource: "qrc:/downloadPageImage"
    layoutSpacing: units.gridUnit
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
    textLevel: 4

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
    RowLayout {
        Layout.alignment: Qt.AlignHCenter
        spacing: 0

        QQC2.Label {
            id: refLabel
            text: {
                if ((currentStatus == Units.DownloadStatus.Failed_Verification ||
                     currentStatus == Units.DownloadStatus.Failed) && !availableDrives)
                    qsTr("Your drive was unplugged during the process")
                else
                    releases.variant.statusString
            }
            visible: text !== downloadPage.text
        }

        QQC2.Label {
            visible: currentStatus == Units.DownloadStatus.Downloading
            text: downloadPage.leftStr
        }

        QQC2.Label {
            visible: currentStatus == Units.DownloadStatus.Downloading
            text: downloadPage.rightStr
        }
    }

    QQC2.ProgressBar {
        id: progressBar
        Layout.fillWidth: true
        value: 0.0
    }

    Column {
        id: infoColumn
        width: downloadPage.width - (units.gridUnit * 8)
        spacing: units.gridUnit / 2

        QQC2.Label {
            id: messageDownload
            visible: currentStatus === Units.DownloadStatus.Downloading ||
                     currentStatus === Units.DownloadStatus.Download_Verifying
            text: qsTr("Downloads are saved to the downloads folder.")
            width: infoColumn.width
            wrapMode: QQC2.Label.Wrap
        }

        QQC2.Label {
            id: messageLoseData
            visible: availableDrives && (currentStatus === Units.DownloadStatus.Failed ||
                                         currentStatus === Units.DownloadStatus.Failed_Verification ||
                                         currentStatus === Units.DownloadStatus.Ready)
            text: qsTr("By writing, you will lose all of the data on %1.").arg(drives.selected.name)
            width: infoColumn.width
            wrapMode: QQC2.Label.Wrap
        }

        QQC2.Label {
            id: messageInsertDrive
            visible: currentStatus === Units.DownloadStatus.Ready && !availableDrives
            text: qsTr("Please insert an USB drive.")
            width: infoColumn.width
            wrapMode: QQC2.Label.Wrap
        }

        QQC2.Label {
            id: messageRestore
            visible: currentStatus === Units.DownloadStatus.Write_Verifying ||
                     currentStatus === Units.DownloadStatus.Writing
            text: qsTr("Your drive will be resized to a smaller capacity. You may resize it back to normal by using Bazzite Media Writer. This will remove installation media from your drive.")
            width: infoColumn.width
            wrapMode: QQC2.Label.Wrap
        }

        QQC2.Label {
            id: messageSelectedImage
            visible: releases.selected.isLocal
            text: "<font color=\"gray\">" + qsTr("Selected:") + "</font> " + (releases.variant.iso ? (((String)(releases.variant.iso)).split("/").slice(-1)[0]) : ("<font color=\"gray\">" + qsTr("None") + "</font>"))
            width: infoColumn.width
            wrapMode: QQC2.Label.Wrap
        }

        QQC2.Label {
            id: messageArmBoard
            visible: false //boardCombo.otherSelected
            text: qsTr("Your board or device is not supported by Bazzite Media Writer yet. Please check <a href=%1>this page</a> for more information about its compatibility with Bazzite and how to create bootable media for it.").arg("https://universal-blue.org")
            width: infoColumn.width
            wrapMode: QQC2.Label.Wrap
        }

        QQC2.Label {
            id: messageDriveSize
            enabled: currentStatus != Units.DownloadStatus.Writing &&
                     currentStatus != Units.DownloadStatus.Write_Verifying &&
                     currentStatus != Units.DownloadStatus.Finished
            visible: enabled && drives.selected && drives.selected.size > 160 * 1024 * 1024 * 1024 // warn when it's more than 160GB
            text: qsTr("The selected drive's size is %1. It's possible you have selected an external drive by accident!").arg(drives.selected ? drives.selected.readableSize : "N/A")
            width: infoColumn.width
            wrapMode: QQC2.Label.Wrap
        }

        QQC2.Label {
            id: messageFinished
            visible: currentStatus === Units.DownloadStatus.Finished
            text: qsTr("Restart and boot from %1 to start using %2.").arg(drives.selected ? drives.selected.name : "N/A").arg(mainWindow.selectedOption == Units.MainSelect.Write ? (String)(releases.localFile.iso).split("/").slice(-1)[0] : releases.selected.name)
            width: infoColumn.width
            wrapMode: QQC2.Label.Wrap
        }

        QQC2.Label {
            visible: releases.variant && releases.variant.errorString.length > 0
            text: releases.variant ? releases.variant.errorString : ""
            width: infoColumn.width
            wrapMode: QQC2.Label.Wrap
        }
    }

    states: [
        State {
            name: "downloading"
            when: currentStatus === Units.DownloadStatus.Downloading
            PropertyChanges {
                target: progressBar;
                value: releases.variant.progress.ratio
            }
        },
        State {
            name: "download_verifying"
            when: currentStatus === Units.DownloadStatus.Download_Verifying
            PropertyChanges {
                target: progressBar;
                value: releases.variant.progress.ratio
            }
        },
        State {
            name: "writing"
            when: currentStatus === Units.DownloadStatus.Writing
            PropertyChanges {
                target: progressBar;
                value: drives.selected.progress.ratio;
            }
        },
        State {
            name: "write_verifying"
            when: currentStatus === Units.DownloadStatus.Write_Verifying
            PropertyChanges {
                target: progressBar;
                value: drives.selected.progress.ratio;
            }
        },
        State {
            name: "finished"
            when: currentStatus === Units.DownloadStatus.Finished
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
        }
    ]    

    // There will be only [Finish] button on the right side so [Cancel] button
    // is not necessary
    previousButtonVisible: currentStatus != Units.DownloadStatus.Finished
    previousButtonText: qsTr("Cancel")
    onPreviousButtonClicked: {
        if (releases.variant.status === Units.DownloadStatus.Write_Verifying ||
            releases.variant.status === Units.DownloadStatus.Writing ||
            releases.variant.status === Units.DownloadStatus.Downloading ||
            releases.variant.status === Units.DownloadStatus.Download_Verifying) {
            cancelDialog.show()
        } else {
            releases.variant.resetStatus()
            downloadManager.cancel()
            mainWindow.selectedPage = Units.Page.MainPage
        }
    }

    nextButtonVisible: {
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
    nextButtonText: {

        if (releases.variant.status === Units.DownloadStatus.Write_Verifying ||
            releases.variant.status === Units.DownloadStatus.Writing ||
            releases.variant.status === Units.DownloadStatus.Downloading ||
            releases.variant.status === Units.DownloadStatus.Download_Verifying)
            return qsTr("Cancel")
        else if (releases.variant.status == Units.DownloadStatus.Ready)
            return qsTr("Write")
        else if (releases.variant.status === Units.DownloadStatus.Finished)
            return qsTr("Finish")
        else
            return qsTr("Retry")
    }
    onNextButtonClicked: {
        if (releases.variant.status === Units.DownloadStatus.Finished) {
            drives.lastRestoreable = drives.selected
            drives.lastRestoreable.setRestoreStatus(Units.RestoreStatus.Contains_Live)
            releases.variant.resetStatus()
            downloadManager.cancel()
            selectedPage = Units.Page.MainPage
        } else if ((releases.variant.status === Units.DownloadStatus.Failed && drives.length) ||
                   (releases.variant.status === Units.DownloadStatus.Failed_Verification && drives.length) ||
                    releases.variant.status === Units.DownloadStatus.Failed_Download ||
                    releases.variant.status === Units.DownloadStatus.Ready) {
            if (selectedOption != Units.MainSelect.Write)
                releases.variant.download()
            drives.selected.setImage(releases.variant)
            drives.selected.write(releases.variant)
        }
    }
}
