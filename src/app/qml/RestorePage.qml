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

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

Page {
    id: restorePage

    property var selectedDrive: null
    property var restoreableDrives: drives.restoreableDrives

    onRestoreableDrivesChanged: {
        if (restoreableDrives.length === 0 && !(selectedDrive && (selectedDrive.restoreStatus === Units.RestoreStatus.Restoring ||
                                                                   selectedDrive.restoreStatus === Units.RestoreStatus.Restored ||
                                                                   selectedDrive.restoreStatus === Units.RestoreStatus.Restore_Error))) {
            selectedPage = Units.Page.MainPage
        }
    }

    text: selectedDrive ? qsTr("Restore Drive <b>%1</b>").arg(selectedDrive.name) : qsTr("Restore Drive")
    textLevel: 1

    ColumnLayout {
        visible: drives.restoreableDrives.length > 1
        Layout.fillWidth: true
        Layout.bottomMargin: units.gridUnit

        Heading {
            text: qsTr("Select a drive to restore")
        }

        QQC2.ComboBox {
            Layout.fillWidth: true
            model: drives.restoreableDrives
            textRole: "name"
            onActivated: restorePage.selectedDrive = drives.restoreableDrives[currentIndex]
        }
    }

    Component.onCompleted: {
        if (drives.restoreableDrives.length > 0) {
            selectedDrive = drives.restoreableDrives[0]
        }
    }

    QQC2.Label {
        id: warningText
        visible: selectedDrive && selectedDrive.restoreStatus == Units.RestoreStatus.Contains_Live
        Layout.alignment: Qt.AlignHCenter
        Layout.fillWidth: true
        text: qsTr("<p align=\"justify\"> To reclaim all space available on the drive, it has to be restored to its factory settings. The live system and all saved data will be deleted.</p> <p align=\"justify\"> You don't need to restore the drive if you want to write another live system to it.</p> <p align=\"justify\"> Do you want to restore it to factory settings? </p>" )
        textFormat: Text.RichText
        wrapMode: QQC2.Label.Wrap
    }

    ColumnLayout {
        id: progress
        visible: selectedDrive && selectedDrive.restoreStatus == Units.RestoreStatus.Restoring

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
            value: selectedDrive ? selectedDrive.progress.ratio : 0
        }
    }

    QQC2.Label {
        id: restoredText
        visible: selectedDrive && selectedDrive.restoreStatus == Units.RestoreStatus.Restored
        Layout.alignment: Qt.AlignHCenter
        Layout.fillWidth: true
        text: qsTr("Your drive was successfully restored!")
        wrapMode: QQC2.Label.Wrap
    }

    QQC2.Label {
        id: errorText
        visible: selectedDrive && selectedDrive.restoreStatus == Units.RestoreStatus.Restore_Error
        Layout.alignment: Qt.AlignHCenter
        Layout.fillWidth: true
        text: qsTr("Unfortunately, an error occurred during the process. Please try restoring the drive using your system tools.")
        wrapMode: QQC2.Label.Wrap
    }

    states: [
        State {
            name: "restored"
            when: selectedDrive && selectedDrive.restoreStatus == Units.RestoreStatus.Restored
            PropertyChanges {
                target: mainWindow
                title: qsTr("Restoring finished")
            }
        }
    ]

    previousButtonVisible: selectedDrive && selectedDrive.restoreStatus != Units.RestoreStatus.Restored &&
                           selectedDrive.restoreStatus != Units.RestoreStatus.Restoring &&
                           selectedDrive.restoreStatus != Units.RestoreStatus.Restore_Error
    onPreviousButtonClicked: {
        selectedPage = Units.Page.MainPage
    }

    nextButtonEnabled: selectedDrive && (selectedDrive.restoreStatus == Units.RestoreStatus.Restored ||
                                         selectedDrive.restoreStatus == Units.RestoreStatus.Restore_Error ||
                                         selectedDrive.restoreStatus == Units.RestoreStatus.Contains_Live)
    nextButtonVisible: selectedDrive && selectedDrive.restoreStatus != Units.RestoreStatus.Restoring
    nextButtonText: selectedDrive && (selectedDrive.restoreStatus == Units.RestoreStatus.Restored ||
                                      selectedDrive.restoreStatus == Units.RestoreStatus.Restore_Error) ? qsTr("Finish") : qsTr("Restore")
    onNextButtonClicked: {
        if (selectedDrive.restoreStatus == Units.RestoreStatus.Restored ||
            selectedDrive.restoreStatus == Units.RestoreStatus.Restore_Error) {
            selectedPage = Units.Page.MainPage
        } else {
            selectedDrive.restore()
        }
    }

}
