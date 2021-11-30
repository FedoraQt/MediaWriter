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
import QtQuick.Layouts 6.2
import QtQml 6.2


Page {
    title: qsTr("Select Fedora Version")
    
    ColumnLayout {
        anchors.fill: parent
        
        Heading {
            Layout.topMargin: units.gridUnit
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("Select Fedora Version")
            level: 5
        }
        
        ColumnLayout {
            Layout.alignment: Qt.AlignLeft
            Layout.topMargin: units.gridUnit / 2
            Layout.leftMargin: units.gridUnit * 9
            Layout.rightMargin: units.gridUnit * 9
            
            Label {
                Layout.alignment: Qt.AlignLeft
                text: qsTr("Select from:")
            }
        
            RadioButton {
                Layout.alignment: Qt.AlignLeft
                checked: true
                text: qsTr("Official Editions")
            }
    
            RadioButton {
                Layout.alignment: Qt.AlignLeft
                text: qsTr("Emerging Editions")
            }
            
            RadioButton {
                Layout.alignment: Qt.AlignLeft
                text: qsTr("Spins")
            }
            
            RadioButton {
                Layout.alignment: Qt.AlignLeft
                text: qsTr("Labs")
            }
            
            ComboBox {
                id: selectFromComboBox
                Layout.alignment: Qt.AlignHCenter
                textRole: "text"
                valueRole: "value"
                Layout.fillWidth: true
                Layout.topMargin: units.gridUnit / 2
                model: [
                    { value: 1, text: "Prvni moznost" },
                    { value: 2, text: "Druha moznost" },
                    { value: 3, text: "Treti moznost" }
                ]
            }
        }
        
        Item {
            Layout.fillHeight: true
        }
        
        RowLayout {
            Layout.topMargin: units.gridUnit * 2
            Layout.leftMargin: units.gridUnit * 3
            Layout.rightMargin: units.gridUnit * 3
            Layout.bottomMargin: units.gridUnit * 2
            Layout.alignment: Qt.AlignBottom
            
            Button {
                id: previousButton
                text: qsTr("Previous")
                onClicked: stackView.pop("VersionPage.qml")
            }
        
            Item {
                Layout.fillWidth: true
            }
            
            Button {
                id: nextButton
                text: qsTr("Next")
                onClicked: stackView.push("DrivePage.qml")
            }
        }
    }
}
