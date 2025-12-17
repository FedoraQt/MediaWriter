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

import QtQuick 6.6
import QtQuick.Controls 6.6
import QtQuick.Window 6.6
import QtQuick.Layouts 6.6

ApplicationWindow {    
    id: aboutDialog
    minimumWidth: Math.max(420, units.gridUnit * 22)
    maximumWidth: Math.max(420, units.gridUnit * 22)
    minimumHeight: Math.max(240, units.gridUnit * 12)
    maximumHeight: Math.max(240, units.gridUnit * 12)
    modality: Qt.ApplicationModal
    x: Screen.width / 2 - width / 2
    y: Screen.height / 2 - height / 2
    title: " "
    
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
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("About Bazzite Media Writer")
                level: 3
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                width: mainColumn.width - units.gridUnit * 2
            }
        
            Label {
                width: mainColumn.width - units.gridUnit * 2
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                text: qsTr("Version %1").arg(mediawriterVersion)
            }
            
            Label {
                width: mainColumn.width - units.gridUnit * 2
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                visible: releases.beingUpdated
                text: qsTr("Bazzite Media Writer is now checking for new releases")
            }
            
            Label {
                width: mainColumn.width - units.gridUnit * 2
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
                onClicked: aboutDialog.close()
                text: qsTr("Close")
            }
        }
    }
}
