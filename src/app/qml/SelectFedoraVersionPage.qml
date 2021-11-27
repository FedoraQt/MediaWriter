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
import QtQuick.Layouts 6.2
import QtQml 6.2


Page {
    title: "Select Fedora Version"
    
    ColumnLayout {
        id: mainColumn
        anchors.fill: parent
        
        Label {
            Layout.alignment: Qt.AlignHCenter
            font.bold: true
            font.pixelSize: units.gridUnit * 1.5
            text: "Select Fedora Version"   
        }
        
        ColumnLayout {
            Layout.alignment: Qt.AlignHCenter
            
            Label {
                Layout.alignment: Qt.AlignHCenter
                font.pixelSize: units.gridUnit
                text: "Select from:"
            }
        
            RadioButton {
                Layout.alignment: Qt.AlignLeft
                checked: true
                text: "Official Editions"
                font.pixelSize: units.gridUnit
            }
    
            RadioButton {
                Layout.alignment: Qt.AlignLeft
                text: "Emerging Editions"
                font.pixelSize: units.gridUnit
            }
            
            RadioButton {
                Layout.alignment: Qt.AlignLeft
                text: "Spins"
                font.pixelSize: units.gridUnit
            }
            
            RadioButton {
                Layout.alignment: Qt.AlignLeft
                font.pixelSize: units.gridUnit
                text: "Labs"
            }
            
            ComboBox {
                id: selectFromComboBox
                Layout.alignment: Qt.AlignHCenter
                textRole: "text"
                valueRole: "value"
                font.pixelSize: units.gridUnit
                model: [
                    { value: 1, text: "Prvni moznost" },
                    { value: 2, text: "Druha moznost" },
                    { value: 3, text: "Treti moznost" }
                ]
            }
        }
        
        RowLayout {
            Layout.margins: units.gridUnit * 1.3
            Layout.alignment: Qt.AlignBottom
            Button {
                id: previousButton
                text: qsTr("Previous")
                font.pixelSize: units.gridUnit
                onClicked: stackView.pop("SelectFedoraVersionPage.qml")
            }
        
            Item {
                Layout.fillWidth: true
            }
            
            Button {
                id: nextButton
                text: qsTr("Next")
                font.pixelSize: units.gridUnit
                onClicked: stackView.push("SelectDrivePage.qml")
            }
        }
    }
}


