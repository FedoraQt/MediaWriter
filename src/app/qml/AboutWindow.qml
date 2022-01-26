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

ApplicationWindow {    
    id: aboutWindow
    visible: mainWindow.visibleAboutWindow
    minimumWidth: mainWindow.width / 2 + units.gridUnit * 5
    minimumHeight: mainWindow.height / 2
    modality: Qt.ApplicationModal
    
    Component.onCompleted: {
        width = mainWindow.minimumWidth / 2 + units.gridUnit * 5
        height = mainWindow.minimumHeight / 2
    }
    
    ColumnLayout {
        id: mainColumn
        anchors.fill: parent
        anchors.leftMargin: units.gridUnit * 3
        anchors.rightMargin: units.gridUnit * 3
        anchors.topMargin: units.gridUnit * 2
        anchors.bottomMargin: units.gridUnit * 2
        spacing: units.gridUnit
            
        Heading {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("About Fedora Media Writer")
            level: 3
        }
        
        Column {
            Label {
                width: mainColumn.width
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                text: qsTr("Version %1").arg(mediawriterVersion)
            }
            
            Label {
                width: mainColumn.width
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                visible: releases.beingUpdated
                text: qsTr("Fedora Media Writer is now checking for new releases")
            }
            
            Label {
                width: mainColumn.width
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                text: qsTr("Please report bugs or your suggestions on %1").arg("<a href=\"https://github.com/FedoraQt/MediaWriter/issues\">https://github.com/FedoraQt/MediaWriter/</a>")
                textFormat: Text.RichText
                onLinkActivated: Qt.openUrlExternally(link)
                opacity: 0.6

                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.NoButton
                    cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
                }
            } 
        }
        
        RowLayout {
            Layout.alignment: Qt.AlignBottom
            
            Item {
                Layout.fillWidth: true
            }
            
            Button {
                id: closeButton
                onClicked: mainWindow.visibleAboutWindow = !mainWindow.visibleAboutWindow
                text: qsTr("Close")
            }
        }
    }
}
