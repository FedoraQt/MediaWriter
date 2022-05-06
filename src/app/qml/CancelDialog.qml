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

ApplicationWindow {
    id: cancelDialog

    minimumWidth: units.gridUnit * 20
    maximumWidth: units.gridUnit * 20
    minimumHeight: units.gridUnit * 10
    maximumHeight: units.gridUnit * 10

    modality: Qt.ApplicationModal
    x: Screen.width / 2 - width / 2
    y: Screen.height / 2 - height / 2
    title: " "
    
    property QtObject drivesSelected: drives.selected
    
    ColumnLayout {
        id: mainColumn
        anchors.fill: parent
        anchors.margins: units.gridUnit 
        spacing: units.gridUnit
        
        Column {
            leftPadding: units.gridUnit
            rightPadding: units.gridUnit
            spacing: units.gridUnit
            
            Heading {
                level: 2
                text: {
                    if (releases.variant.status == Units.DownloadStatus.Downloading || releases.variant.status === Units.DownloadStatus.Download_Verifying)
                        qsTr("Cancel Download?")
                    else if (releases.variant.status == Units.DownloadStatus.Writing) 
                        qsTr("Cancel Writing?")
                    else
                        qsTr("Cancel Verification?")
                } 
            }
            
            Label {
                text: {
                    if (releases.variant.status == Units.DownloadStatus.Downloading || releases.variant.status === Units.DownloadStatus.Download_Verifying)
                        qsTr("Download progress will be lost and media writing will be aborted.")
                    else if (releases.variant.status == Units.DownloadStatus.Writing) 
                        qsTr("Writing process will be aborted and your drive will have to be restored afterwards.")
                    else
                        qsTr("This operation is safe to be cancelled.")
                }   
                wrapMode: Label.Wrap
                width: mainColumn.width - units.gridUnit * 2
            }
        }
          
        RowLayout {
            Layout.alignment: Qt.AlignBottom
            
            Item {
                Layout.fillWidth: true
            }
            
            Button {
                id: continueButton
                onClicked: cancelDialog.close()
                text: qsTr("Continue")
            }
            
            Button {
                id: cancelButton
                onClicked: {
                    cancelDialog.close()
                    if (mainWindow.selectedPage == Units.Page.DownloadPage && (releases.variant.status === Units.DownloadStatus.Writing || releases.variant.status === Units.DownloadStatus.Write_Verifying || releases.variant.status === Units.DownloadStatus.Writing_Not_Possible)) {
                        drives.selected.cancel()
                        drives.lastRestoreable = drivesSelected
                        drives.lastRestoreable.setRestoreStatus(Units.RestoreStatus.Contains_Live)
                    }
                    releases.variant.resetStatus()
                    downloadManager.cancel()
                    selectedPage = Units.Page.MainPage
                }
                text: {
                    if (releases.variant.status == Units.DownloadStatus.Downloading || releases.variant.status === Units.DownloadStatus.Download_Verifying)
                        qsTr("Cancel Download")
                    else if (releases.variant.status == Units.DownloadStatus.Writing) 
                        qsTr("Cancel Writing")
                    else if (releases.variant.status == Units.DownloadStatus.Write_Verifying) 
                        qsTr("Cancel Verification")
                }  
            }
        }
    }
}

