/*
 * Fedora Media Writer
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
import QtQuick.Layouts
import QtQuick.Templates as T
import org.fedoraproject.AdwaitaTheme 
import "private" as Private

T.ScrollBar {
    id: control

    implicitWidth: implicitContentWidth + leftPadding + rightPadding
    implicitHeight: implicitContentHeight + topPadding + bottomPadding

    leftPadding: 4
    rightPadding: 4
    bottomPadding: 4
    topPadding: 4
    // FIXME
    // Workaround for me not being able to hide horizontal scrollbar
    visible: control.size < 1.0 && orientation !== Qt.Horizontal
    minimumSize: orientation == Qt.Horizontal ? height / width : width / height
    hoverEnabled: true

    contentItem: Rectangle {
        implicitWidth: control.pressed || control.hovered ? 6 : 2
        implicitHeight: control.pressed || control.hovered ? 6 : 2
        radius: width / 2
        color: theme.getScrollBarHandleColor(control.hovered, control.pressed)
    }

    background: Item {
        visible: control.size < 1.0 && (control.pressed || control.hovered)
        implicitWidth: implicitWidth
        implicitHeight: implicitWidth
        Rectangle {
            anchors {
                top: parent.top
                bottom: parent.bottom
                left: parent.left
            }
            width: parent.width
            color: theme.getScrollBarGrooveColor()
            border.color: Qt.darker(palette.window, 1.2) // FIXME
            border.width: 1
        }
    }
}
