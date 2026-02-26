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
 * This program is in the hope that it will be useful,
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
    id: restorePage

    property var selectedDrive: drives.restoreableCount > 0 ? drives.restoreableDriveAt(driveCombo.currentIndex) : null
    property var restoringDrive: null

    text: qsTr("Restore Drive <b>%1</b>").arg(restoringDrive ? restoringDrive.name : (selectedDrive ? selectedDrive.name : ""))
    textLevel: 1

    ColumnLayout {
        id: driveSelectionColumn
        visible: drives.restoreableCount > 0 && !restoringDrive
        Layout.fillWidth: true

        QQC2.Label {
            text: qsTr("Select USB Drive to Restore:")
            font.bold: true
        }

        QQC2.ComboBox {
            id: driveCombo
            Layout.fillWidth: true
            model: drives.restoreableDrives
            enabled: drives.restoreableCount > 0
            visible: drives.restoreableCount > 0
            currentIndex: 0
            textRole: "name"
        }
    }

    QQC2.Label {
        id: warningText
        visible: selectedDrive && selectedDrive.restoreStatus == Units.RestoreStatus.Contains_Live && !restoringDrive
        Layout.alignment: Qt.AlignHCenter
        Layout.fillWidth: true
        text: qsTr("<p align=\"justify\"> To reclaim all space available on the drive, it has to be restored to its factory settings. The live system and all saved data will be deleted.</p> <p align=\"justify\"> You don't need to restore the drive if you want to write another live system to it.</p> <p align=\"justify\"> Do you want to restore it to factory settings? </p>" )
        textFormat: Text.RichText
        wrapMode: QQC2.Label.Wrap
    }

    ColumnLayout {
        id: progress
        visible: restoringDrive && restoringDrive.restoreStatus == Units.RestoreStatus.Restoring

        Layout.alignment: Qt.AlignHCenter
        Layout.fillWidth: true

        QQC2.Label {
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            horizontalAlignment: QQC2.Label.AlignHCenter
            wrapMode: QQC2.Label.Wrap
            text: qsTr("<p align=\"justify\">Please wait while Fedora Media Writer restores your portable drive.</p>")
        }

        QQC2.ProgressBar {
            id: progressIndicator
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            indeterminate: true
        }
    }

    QQC2.Label {
        id: restoredText
        visible: restoringDrive && restoringDrive.restoreStatus == Units.RestoreStatus.Restored
        Layout.alignment: Qt.AlignHCenter
        Layout.fillWidth: true
        text: qsTr("Your drive was successfully restored!")
        wrapMode: QQC2.Label.Wrap
    }

    QQC2.Label {
        id: errorText
        visible: restoringDrive && restoringDrive.restoreStatus == Units.RestoreStatus.Restore_Error
        Layout.alignment: Qt.AlignHCenter
        Layout.fillWidth: true
        text: qsTr("Unfortunately, an error occurred during the process. Please try restoring the drive using your system tools.")
        wrapMode: QQC2.Label.Wrap
    }

    states: [
        State {
            name: "restored"
            when: restoringDrive && restoringDrive.restoreStatus == Units.RestoreStatus.Restored
            PropertyChanges {
                target: mainWindow;
                title: qsTr("Restoring finished")
            }
        }
    ]

    previousButtonEnabled: !restoringDrive || (restoringDrive.restoreStatus != Units.RestoreStatus.Restoring && restoringDrive.restoreStatus != Units.RestoreStatus.Restored)
    previousButtonVisible: previousButtonEnabled
    onPreviousButtonClicked: {
        restoringDrive = null
        selectedPage = Units.Page.MainPage
    }

    nextButtonEnabled: {
        if (restoringDrive) {
            return restoringDrive.restoreStatus == Units.RestoreStatus.Restored || 
                   restoringDrive.restoreStatus == Units.RestoreStatus.Restore_Error
        }
        return selectedDrive && selectedDrive.restoreStatus == Units.RestoreStatus.Contains_Live
    }
    nextButtonVisible: !restoringDrive || restoringDrive.restoreStatus != Units.RestoreStatus.Restoring
    nextButtonText: restoringDrive && (restoringDrive.restoreStatus == Units.RestoreStatus.Restored || restoringDrive.restoreStatus == Units.RestoreStatus.Restore_Error) ? qsTr("Finish") : qsTr("Restore")
    onNextButtonClicked: {
        if (restoringDrive && (restoringDrive.restoreStatus == Units.RestoreStatus.Restored || restoringDrive.restoreStatus == Units.RestoreStatus.Restore_Error)) {
            restoringDrive = null
            selectedPage = Units.Page.MainPage
        } else if (selectedDrive) {
            restoringDrive = selectedDrive
            restoringDrive.restore()
        }
    }
}
