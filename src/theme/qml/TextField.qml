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
import "private" as Private

T.TextField {
    id: control

    implicitWidth: implicitBackgroundWidth + leftInset + rightInset
                   || Math.max(contentWidth, placeholder.implicitWidth) + leftPadding + rightPadding
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding,
                             placeholder.implicitHeight + topPadding + bottomPadding)

    leftPadding: theme.lineEditMarginWidth + theme.frameWidth
    rightPadding: theme.lineEditMarginWidth + theme.frameWidth
    bottomPadding: theme.lineEditMarginHeight + theme.frameWidth
    topPadding: theme.lineEditMarginHeight + theme.frameWidth

    color: theme.textColor
    selectionColor: theme.highlightColor
    selectedTextColor: theme.highlightTextColor
    placeholderTextColor: Qt.hsla(control.color.hslHue, control.color.hslSaturation, control.color.hslLightness, 0.6)
    verticalAlignment: TextInput.AlignVCenter

    Label {
        id: placeholder
        x: control.leftPadding
        y: control.topPadding
        width: control.width - (control.leftPadding + control.rightPadding)
        height: control.height - (control.topPadding + control.bottomPadding)

        text: control.placeholderText
        font: control.font
        color: control.placeholderTextColor
        verticalAlignment: control.verticalAlignment
        visible: !control.length && !control.preeditText && (!control.activeFocus || control.horizontalAlignment !== Qt.AlignHCenter)
        elide: Text.ElideRight
        renderType: control.renderType
    }

    background: Item {
        implicitWidth: theme.lineEditMinimumWidth
        implicitHeight: theme.lineEditMinimumHeight

        Rectangle {
            anchors.fill: parent
            anchors.margins: 1
            border.color: control.focus ? theme.highlightColor : theme.getButtonOutlineColor(false, false, control.hovered, false)
            color: control.enabled ? theme.baseColor :theme.windowColor
            radius: theme.frameRadius
        }
    }
}
