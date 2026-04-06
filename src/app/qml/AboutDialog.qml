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

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

QQC2.Dialog {
    id: aboutDialog
    parent: QQC2.Overlay.overlay
    x: parent ? Math.round((parent.width - width) / 2) : 0
    y: parent ? Math.round((parent.height - height) / 2) : 0
    width: Math.max(420, units.gridUnit * 22)
    height: Math.max(240, units.gridUnit * 12)
    modal: true
    focus: true
    closePolicy: QQC2.Popup.CloseOnEscape
    padding: units.gridUnit
    header: Item {
        visible: false
        implicitWidth: 0
        implicitHeight: 0
    }

    contentItem: ColumnLayout {
        id: mainColumn
        spacing: units.gridUnit

        Column {
            leftPadding: units.gridUnit
            rightPadding: units.gridUnit
            spacing: units.gridUnit

            Heading {
                text: qsTr("About Fedora Media Writer")
                level: 3
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                width: mainColumn.width - units.gridUnit * 2
            }

            QQC2.Label {
                width: mainColumn.width - units.gridUnit * 2
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                text: qsTr("Version %1").arg(mediawriterVersion)
            }

            QQC2.Label {
                width: mainColumn.width - units.gridUnit * 2
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                visible: releases.beingUpdated
                text: qsTr("Fedora Media Writer is now checking for new releases")
            }

            QQC2.Label {
                width: mainColumn.width - units.gridUnit * 2
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                text: qsTr("Please report bugs or your suggestions on %1").arg("<a href=\"https://github.com/FedoraQt/MediaWriter/issues\"><font color=\"#0b57d0\">https://github.com/FedoraQt/MediaWriter/</font></a>")
                textFormat: Text.RichText
                onLinkActivated: Qt.openUrlExternally(link)

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

            QQC2.Button {
                id: closeButton
                onClicked: aboutDialog.close()
                text: qsTr("Close")
            }
        }
    }
}
