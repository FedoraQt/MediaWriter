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

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Templates 2.12 as T
import AdwaitaTheme 2.0
import "private" as Private

T.Button {
    id: control

    implicitWidth: contentItem.implicitWidth + (2 * theme.buttonMarginWidth) + (2 * theme.frameWidth)
    implicitHeight: contentItem.implicitHeight + (2 * theme.buttonMarginHeight) + (2 * theme.frameWidth)

    Layout.minimumHeight: theme.buttonMinimumHeight
    Layout.minimumWidth: theme.buttonMinimumWidth

    hoverEnabled: true

    contentItem: RowLayout {
        Label {
            id: label
            Layout.fillWidth: true
            text: control.text
            font: control.font
            color: control.highlighted ? theme.highlightTextColor : control.enabled ? theme.textColor : theme.disabledTextColor
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    background: Item {
        Private.ButtonFrame {
            anchors.fill: parent
            flat: control.flat
            highlighted: control.highlighted
        }
    }
}

