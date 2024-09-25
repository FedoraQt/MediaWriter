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

    Column {
        QQC2.Label {
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
            text: qsTr("Restore Drive <b>%1</b>").arg(lastRestoreable.name)
            wrapMode: QQC2.Label.Wrap
            width: mainWindow.width - (units.gridUnit * 4)
            horizontalAlignment: QQC2.Label.AlignHCenter
        }
    }

    Column {
        QQC2.Label {
            id: warningText
            visible: lastRestoreable.restoreStatus == Units.RestoreStatus.Contains_Live
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("<p align=\"justify\"> To reclaim all space available on the drive, it has to be restored to its factory settings. The live system and all saved data will be deleted. </p> <p align=\"justify\"> You don't need to restore the drive if you want to write another live system to it.
            </p> <p align=\"justify\"> Do you want to restore it to factory settings? </p>" )
            textFormat: Text.RichText
            horizontalAlignment: Text.AlignHCenter
            wrapMode: QQC2.Label.Wrap
            width: mainWindow.width - (units.gridUnit * 4)
        }

        ColumnLayout {
            id: progress
            visible: lastRestoreable.restoreStatus == Units.RestoreStatus.Restoring

            Column{
                QQC2.Label {
                    Layout.alignment: Qt.AlignHCenter
                    horizontalAlignment: QQC2.Label.AlignHCenter
                    wrapMode: QQC2.Label.Wrap
                    width: warningText.width
                    text: qsTr("<p align=\"justify\">Please wait while Fedora Media Writer restores your portable drive.</p>")
                }
            }

            QQC2.ProgressBar {
                id: progressIndicator
                width: units.gridUnit * 14
                Layout.alignment: Qt.AlignHCenter
                indeterminate: true
            }
        }

        QQC2.Label {
            id: restoredText
            visible: lastRestoreable.restoreStatus == Units.RestoreStatus.Restored
            Layout.alignment: Qt.AlignHCenter
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("Your drive was successfully restored!")
            wrapMode: QQC2.Label.Wrap
            width: mainWindow.width - (units.gridUnit * 4)
        }

        QQC2.Label {
            id: errorText
            visible: lastRestoreable.restoreStatus == Units.RestoreStatus.Restore_Error
            Layout.alignment: Qt.AlignHCenter
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("Unfortunately, an error occurred during the process. Please try restoring the drive using your system tools.")
            wrapMode: QQC2.Label.Wrap
            width: mainWindow.width - (units.gridUnit * 4)
        }
    }
    
    Component.onCompleted: {
        lastRestoreable = drives.lastRestoreable
    }
    
    states: [
        State {
            name: "restored"
            when: lastRestoreable.restoreStatus == Units.RestoreStatus.Restored
            PropertyChanges {
                target: mainWindow;
                title: qsTr("Restoring finished")
            }
            StateChangeScript {
                script: drives.lastRestoreable = null
            }
        }
    ]

    previousButtonEnabled: lastRestoreable.restoreStatus != Units.RestoreStatus.Restored &&
                           lastRestoreable.restoreStatus != Units.RestoreStatus.Restoring
    previousButtonVisible: previousButtonEnabled
    onPreviousButtonClicked: {
        selectedPage = Units.Page.MainPage
    }

    nextButtonEnabled: lastRestoreable.restoreStatus == Units.RestoreStatus.Restored ||
                       lastRestoreable.restoreStatus == Units.RestoreStatus.Contains_Live
    nextButtonVisible: lastRestoreable.restoreStatus != Units.RestoreStatus.Restoring
    nextButtonText: lastRestoreable.restoreStatus == Units.RestoreStatus.Restored ? qsTr("Finish") : qsTr("Restore")
    onNextButtonClicked: {
        if (lastRestoreable.restoreStatus == Units.RestoreStatus.Restored)
            selectedPage = Units.Page.MainPage
        else
            drives.lastRestoreable.restore()
    }

}
