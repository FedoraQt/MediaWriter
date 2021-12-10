/*
 * Fedora Media Writer
 * Copyright (C) 2021 Evžen Gasta <evzen.ml@seznam.cz>
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
    ColumnLayout {
        anchors.fill: parent
        spacing: units.gridUnit
        
        Image {
            source: "qrc:/downloadPageImage"
            Layout.fillHeight: true
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectFit
        }
        
        Heading {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("Downloading Fedora Workstation 35")
            level: 5
        }
        
        ColumnLayout {
            Label {
                Layout.alignment: Qt.AlignHCenter
                text: "0.3 of 1.9 GB downloaded"
            }
        
            ProgressBar {
                id: progressBar
                Layout.topMargin: units.gridUnit / 2
                Layout.fillWidth: true
                value: 0.0
            }
        }
        
        //ColumnLayout {
            //Layout.alignment: Qt.AlignHCenter 
            //Layout.topMargin: units.gridUnit / 2
            
            //Column {
                //InfoMessage {
                    //id: messageRestore
                    //visible: false
                    //text: qsTr("Your drive will be resized to a smaller capacity. You may resize it back to normal by using Fedora Media Writer; this will remove installation media from your drive.")
                    //width: layout.width
                //}
                    
                //InfoMessage {
                    //id: messageDriveSize
                    //enabled: true
                    //visible: enabled && drives.selected && drives.selected.size > 160 * 1024 * 1024 * 1024 // warn when it's more than 160GB
                    //text: qsTr("The selected drive's size is %1. It's possible you have selected an external drive by accident!").arg(drives.selected ? drives.selected.readableSize : "N/A")
                    //width: layout.width
                //}
            //}
        //}
    }
    
    
    
    states: [
        State {
            name: "writing"
            when: releases.variant.status === Variant.WRITING
            PropertyChanges {
                target: messageDriveSize
                enabled: false
            }
            PropertyChanges {
                target: messageRestore;
                visible: true
            }
            PropertyChanges {
                target: driveCombo;
                enabled: false
            }
            PropertyChanges {
                target: progressBar;
                value: drives.selected.progress.ratio;
            }
            StateChangeScript {
                name: "colorChange"
                script:  {
                    if (progressBar.hasOwnProperty("destructiveAction")) {
                        progressBar.destructiveAction = false
                    }
                    if (progressBar.hasOwnProperty("progressBarColor")) {
                        progressBar.progressBarColor = "red"
                    }
                }
            }
        },
        State {
            name: "write_verifying"
            when: releases.variant.status === Variant.WRITE_VERIFYING
            PropertyChanges {
                target: messageDriveSize
                enabled: false
            }
            PropertyChanges {
                target: messageRestore;
                visible: true
            }
            //PropertyChanges {
                //target: driveCombo;
                //enabled: false
            //}
            PropertyChanges {
                target: progressBar;
                value: drives.selected.progress.ratio;
            }
            StateChangeScript {
                name: "colorChange"
                script:  {
                    if (progressBar.hasOwnProperty("progressBarColor")) {
                        progressBar.progressBarColor = Qt.lighter("green")
                    }
                }
            }
        },
        State {
            name: "finished"
            when: releases.variant.status === Variant.FINISHED
            PropertyChanges {
                target: messageDriveSize
                enabled: false
            }
            PropertyChanges {
                target: messageRestore;
                visible: true
            }
            //PropertyChanges {
                //target: leftButton;
                //text: qsTr("Close");
                //highlighted: true
                //onClicked: {
                    //dialog.close()
                //}
            //}
            //PropertyChanges {
                //target: deleteButton
                //state: "ready"
            //}
        }
    ]
}

