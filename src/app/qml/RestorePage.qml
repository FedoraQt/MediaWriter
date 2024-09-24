/*
 * Fedora Media Writer
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
import QtQuick.Controls 6.6
import QtQuick.Window 6.6
import QtQuick.Layouts 6.6
import QtQml 6.6

Page {  
    id: restorePage
    
    ColumnLayout {
        id: mainColumn
        anchors.fill: parent
        spacing: units.gridUnit
        
        Column {
            Label {
                Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                text: qsTr("Restore Drive <b>%1</b>").arg(mainWindow.lastRestoreable.name)
                wrapMode: Label.Wrap
                width: mainColumn.width
                horizontalAlignment: Label.AlignHCenter
            }
        }
        
        Column {
            Label {
                id: warningText
                visible: false
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("<p align=\"justify\"> To reclaim all space available on the drive, it has to be restored to its factory settings. The live system and all saved data will be deleted. </p> <p align=\"justify\"> You don't need to restore the drive if you want to write another live system to it. 
                </p> <p align=\"justify\"> Do you want to restore it to factory settings? </p>" )
                textFormat: Text.RichText
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Label.Wrap
                width: mainColumn.width
            }
            
            ColumnLayout {
                id: progress
                visible: false
                
                Column{
                    Label {
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Label.AlignHCenter
                        wrapMode: Label.Wrap
                        width: warningText.width
                        text: qsTr("<p align=\"justify\">Please wait while Fedora Media Writer restores your portable drive.</p>")
                    }
                }
                
                ProgressBar {
                    id: progressIndicator
                    width: units.gridUnit * 14
                    Layout.alignment: Qt.AlignHCenter
                    indeterminate: true
                }
            }
            
            Label {
                id: restoredText
                visible: false
                Layout.alignment: Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                text: qsTr("Your drive was successfully restored!")
                wrapMode: Label.Wrap
                width: mainColumn.width
            }
            
            Label {
                id: errorText
                visible: false
                Layout.alignment: Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                text: qsTr("Unfortunately, an error occurred during the process. Please try restoring the drive using your system tools.")
                wrapMode: Label.Wrap
                width: mainColumn.width
            }
        }
    }
    
    Connections {
        target: drives
        function onLastRestoreableChanged() {
            if (!drives.selected)
                mainWindow.selectedPage = Units.Page.MainPage 
        }
    }
    
    Component.onCompleted: {
        mainWindow.lastRestoreable = drives.lastRestoreable
    }
    
    states: [
        State {
            name: "contains_live"
            when: mainWindow.lastRestoreable && mainWindow.lastRestoreable.restoreStatus == Units.RestoreStatus.Contains_Live
            PropertyChanges {
                target: warningText;
                visible: true
            }
        },
        State {
            name: "restoring"
            when: mainWindow.lastRestoreable && mainWindow.lastRestoreable.restoreStatus == Units.RestoreStatus.Restoring
            PropertyChanges {
                target: progress;
                visible: true
            }
            PropertyChanges {
                target: prevButton;
                visible: false
            }
            PropertyChanges {
                target: nextButton;
                visible: false
            }
        },
        State {
            name: "restored"
            when: mainWindow.lastRestoreable && mainWindow.lastRestoreable.restoreStatus == Units.RestoreStatus.Restored
            PropertyChanges {
                target: mainWindow;
                title: qsTr("Restoring finished")
            }
            PropertyChanges {
                target: restoredText;
                visible: true
            }
            PropertyChanges {
                target: nextButton;
                enabled: true
            }
            PropertyChanges {
                target: prevButton;
                visible: false
            }
            StateChangeScript {
                script: drives.lastRestoreable = null
            }
        },
        State {
            name: "restore_error"
            when: mainWindow.lastRestoreable && mainWindow.lastRestoreable.restoreStatus == Units.RestoreStatus.Restore_Error
            PropertyChanges {
                target: errorText;
                visible: true
            }
            PropertyChanges {
                target: prevButton;
                enabled: true
            }
        },
        State {
            name: "Clean"
            when: mainWindow.lastRestoreable && mainWindow.lastRestoreable.restoreStatus == Units.RestoreStatus.Clean
            PropertyChanges {
                target: prevButton;
                enabled: true
            }
        }
    ]
}
