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
import QtQuick.Templates as T
import org.fedoraproject.AdwaitaTheme

T.CheckBox {
    id: control

    // FIXME: windows seems to need more space for some reason
    implicitWidth: implicitContentWidth + leftPadding + rightPadding + (Qt.platform.os == "windows" ? indicator.width : spacing)
    implicitHeight: implicitContentHeight + bottomPadding + topPadding

    padding: theme.checkboxItemSpacing
    spacing: theme.checkboxItemSpacing
    hoverEnabled: true

    indicator: CheckIndicator {
        x: control.text ? (control.mirrored ? control.width - width - control.rightPadding : control.leftPadding) : control.leftPadding + (control.availableWidth - width) / 2
        y: control.topPadding + (control.availableHeight - height) / 2
        control: control
    }

    contentItem: Text {
        id: label
        leftPadding: control.indicator && !control.mirrored ? control.indicator.width + control.spacing : 0
        rightPadding: control.indicator && control.mirrored ? control.indicator.width + control.spacing : 0
        text: control.text
        font: control.font
        color: control.enabled ? theme.textColor : theme.disabledTextColor
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        visible: control.text
    }
}
