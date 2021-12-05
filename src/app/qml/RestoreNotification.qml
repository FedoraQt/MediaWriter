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
import QtQuick.Dialogs 6.2

Rectangle {
    id: root
    height: units.gridUnit * 5
    enabled: open
    focus: open
    
    signal accepted
    signal cancelled
    property bool open: false
    property alias text: label.text
    property alias acceptText: buttonAccept.text

    Keys.onEscapePressed: open = false
    
    ColumnLayout {
        anchors.fill: parent
        
        Label {
            id: label
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("You inserted <b>%1</b> that already contains a live system.<br>Do you want to restore it to factory settings?").arg(drives.lastRestoreable.name)
        }
        
        RowLayout {
            anchors.bottomMargin: units.gridUnit / 4
            spacing: units.gridUnit
            
            Item {
                Layout.fillWidth: true
            }
                
            Button {
                id: buttonCancel
                Layout.alignment: Qt.AlignBottom
                text: qsTr("Cancel")
                onClicked: {
                    root.open = false
                    root.cancelled()
                }
            }
            
            Button {
                id: buttonAccept
                Layout.rightMargin: units.gridUnit * 3
                Layout.alignment: Qt.AlignBottom
                text: qsTr("Restore")
                onClicked: {
                    if (drives.lastRestoreable) {
                        drives.lastRestoreable.restore()
                        root.visible = false
                        //root.visible = drives.lastRestoreable
                    }
                    else
                        root.visible = false
                }
            }
        }
    }
}
