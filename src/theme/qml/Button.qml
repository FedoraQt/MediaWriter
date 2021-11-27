/*
 * Fedora Media Writer
 * Copyright (C) 2016 Martin Bříza <mbriza@redhat.com>
 * Copyright (C) 2020 Jan Grulich <jgrulich@redhat.com>
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
import QtQuick.Controls
import QtQuick.Controls.impl
import QtQuick.Templates as T
import QtQuick.Layouts
import org.fedoraproject.AdwaitaTheme
import "private" as Private

T.Button {
    id: control

    property bool destructiveAction: false

    implicitWidth: contentItem.implicitWidth + leftPadding + rightPadding
    implicitHeight: contentItem.implicitHeight + bottomPadding + topPadding

    leftPadding: theme.buttonMarginWidth + theme.frameWidth
    rightPadding: theme.buttonMarginWidth + theme.frameWidth
    bottomPadding: theme.buttonMarginWidth + theme.frameWidth
    topPadding: theme.buttonMarginWidth + theme.frameWidth

    Layout.minimumWidth: control.text ? theme.buttonMinimumWidth : theme.buttonMinimumWidth / 2

    hoverEnabled: true

    contentItem: RowLayout {
         Icon {
            id: icon

            Layout.alignment: Qt.AlignCenter

            Layout.fillWidth: true
            Layout.fillHeight: true

            Layout.minimumWidth: Math.min(parent.width, parent.height, implicitWidth)
            Layout.minimumHeight: Math.min(parent.width, parent.height, implicitHeight)

            Layout.maximumWidth: control.icon.width > 0 ? control.icon.width : Number.POSITIVE_INFINITY
            Layout.maximumHeight: control.icon.height > 0 ? control.icon.height : Number.POSITIVE_INFINITY

            visible: source.length > 0 && control.display !== T.Button.TextOnly
            source: control.icon ? (control.icon.name || control.icon.source) : ""
        }

        Label {
            id: label
            Layout.leftMargin: theme.buttonItemSpacing
            Layout.rightMargin: theme.buttonItemSpacing
            Layout.fillWidth: true
            text: control.text
            font: control.font
            color: control.enabled ? control.highlighted ? theme.highlightTextColor : theme.textColor : theme.disabledTextColor
            horizontalAlignment: icon.visible ? Text.AlignLeft : Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
            visible: control.text
        }
    }

    background: Private.ButtonFrame {
        anchors.fill: parent
        flat: control.flat
        highlighted: control.highlighted
        destructiveAction: control.destructiveAction
    }
}

