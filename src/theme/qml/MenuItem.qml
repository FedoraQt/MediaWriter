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
import QtQuick.Layouts 
import QtQuick.Shapes 
import QtQuick.Templates as T
import org.fedoraproject.AdwaitaTheme 
import "private" as Private

T.MenuItem {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentItem.implicitWidth + leftPadding + rightPadding + spacing + (arrow ? arrow.implicitWidth : 0))
    implicitHeight: Math.max(background ? background.implicitHeight : 0,
                             contentItem.implicitHeight+  topPadding + bottomPadding,
                             indicator ? indicator.implicitHeight : 0)

    bottomPadding: theme.menuItemMarginWidth
    topPadding: theme.menuItemMarginWidth
    leftPadding: theme.menuItemMarginWidth
    rightPadding: theme.menuItemMarginWidth

    spacing: theme.menuItemSpacing

    icon.width: 16
    icon.height: 16

    contentItem:  RowLayout {
        Icon {
            id: icon

            Layout.alignment: Qt.AlignCenter

            Layout.fillWidth: true
            Layout.fillHeight: true

            Layout.minimumWidth: Math.min(parent.width, parent.height, implicitWidth)
            Layout.minimumHeight: Math.min(parent.width, parent.height, implicitHeight)

            Layout.maximumWidth: control.icon.width > 0 ? control.icon.width : Number.POSITIVE_INFINITY
            Layout.maximumHeight: control.icon.height > 0 ? control.icon.height : Number.POSITIVE_INFINITY

            visible: source.length > 0
            source: control.icon ? (control.icon.name || control.icon.source) : ""
        }

        Label {
            id: label
            Layout.leftMargin: control.leftPadding
            Layout.fillWidth: true
            text: control.text
            font: control.font
            color: control.highlighted ? theme.highlightTextColor : control.enabled ? theme.textColor : theme.disabledTextColor
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
            visible: control.text
        }
    }

    arrow: Shape {
        implicitHeight: 10
        implicitWidth: 10
        x: control.mirrored ? control.leftPadding : control.width - width - control.rightPadding
        y: control.topPadding + control.availableHeight / 2
        visible: control.subMenu

        ShapePath {
            fillColor: theme.textColor
            strokeColor: theme.textColor
            capStyle: ShapePath.FlatCap
            joinStyle: ShapePath.MiterJoin

            startX: -2
            startY: -4
            PathLine { x: 2; y: 0 }
            PathLine { x: -2; y: 4 }
        }
    }

    indicator: CheckIndicator {
        x: control.text ? (control.mirrored ? control.width - width - control.rightPadding : control.leftPadding) : control.leftPadding + (control.availableWidth - width) / 2
        y: control.topPadding + (control.availableHeight - height) / 2
        control: control
        visible: control.checkable
    }

    background: Rectangle {
        implicitWidth: 200
        implicitHeight: 20

        color: theme.highlightColor
        visible: control.down || control.highlighted
    }
}
