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

Dialog {
    id: main
    opacity: 0.9
    width: parent.width
    
    ColumnLayout {
        anchors.fill: parent
        spacing: units.gridUnit 
        
        Keys.onEscapePressed: {
            if (drives.lastRestoreable.restoreStatus != Drive.RESTORING)
                root.close()
        }
        
        Heading {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("About")
            level: 5
            color: "white"
        }
        
        Label {
            id: text
            Layout.alignment: Qt.AlignHCenter
            text: "Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Quisque porta. Nullam dapibus fermentum ipsum. \n Ut tempus purus at lorem. Mauris elementum mauris vitae tortor. Nunc auctor. Nulla est."
            color: "white"
        }
        
        RowLayout {
            Layout.topMargin: units.gridUnit * 2
            Layout.leftMargin: units.gridUnit * 2
            Layout.rightMargin: units.gridUnit * 2
            Layout.bottomMargin: units.gridUnit 
            Layout.alignment: Qt.AlignBottom
            
            Item {
                Layout.fillWidth: true
            }
            
            Button {
                id: cancelButton
                text: qsTr("Close")
                onClicked: main.close()
            }
        }
    }
}
