/*
 * Fedora Media Writer
 * Copyright (C) 2021 Evžen Gasta 
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
    title: "Downloading"
    
    ColumnLayout {
        anchors.fill: parent
        
        Image {
            Layout.alignment: Qt.AlignHCenter 
            source: "qrc:/download"
            sourceSize.width: units.gridUnit * 13
            sourceSize.height: units.gridUnit * 13
        }
        
        Label {
            Layout.alignment: Qt.AlignHCenter 
            font.bold: true
            font.pixelSize: units.gridUnit * 1.5
            text: "Downloading Fedora Workstation 35"
        }
        
        ColumnLayout {
            Layout.alignment: Qt.AlignHCenter   
            Label {
                Layout.alignment: Qt.AlignHCenter
                font.pixelSize: units.gridUnit
                text: "0.3 of 1.9 GB downloaded"
            }
        
            ProgressBar {
                Layout.fillWidth: true
                Layout.leftMargin: units.gridUnit * 10
                Layout.rightMargin: units.gridUnit * 10
            }
        }
        
        Label {
            Layout.alignment: Qt.AlignHCenter 
            font.pixelSize: units.gridUnit * 0.7
            text: "Image will be writen to <disk> when download completes"
        }
        
        RowLayout {
            Layout.margins: units.gridUnit * 1.3
            Layout.alignment: Qt.AlignBottom
            
            Item {
                Layout.fillWidth: true
            }
            
            Button {
                id: cancelButton
                font.pixelSize: units.gridUnit
                text: "Cancel"
            }
            
        }
        
    }
}

