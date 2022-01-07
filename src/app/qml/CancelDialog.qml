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
    id: cancelDialog
    visible: mainWindow.visibleCancelDialog
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2 + units.gridUnit * 2
    
    contentItem: Rectangle {
        implicitHeight: mainWindow.height - units.gridUnit * 4
        implicitWidth: mainWindow.width - units.gridUnit * 4
        color: palette.window
        
        ColumnLayout {
            anchors.fill: parent
            spacing: units.gridUnit
            
            Heading {
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("Do you want to cancel writing?")
                level: 3
            }
                
            RowLayout {
                Layout.alignment: Qt.AlignBottom
                Button {
                    id: backButton
                    text: qsTr("Back")
                    onClicked: {
                        mainWindow.visibleCancelDialog = !visibleCancelDialog
                    }
                }
                
                Item {
                    Layout.fillWidth: true
                }
                
                Button {
                    id: cancelButton
                    text: qsTr("Cancel write")
                    onClicked: {
                        drives.selected.cancel()
                        releases.variant.resetStatus()
                        downloadManager.cancel()
                        mainWindow.selectedPage = Units.Page.MainPage
                        mainWindow.visibleCancelDialog = !visibleCancelDialog
                    }
                }
            }
        }
    }
    
    Keys.onEscapePressed: {
        mainWindow.visibleCalcelDialog = !visibleCancelDialog
    }
}

