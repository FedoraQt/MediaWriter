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
import org.fedoraproject.AdwaitaTheme 

Rectangle {
    id: root

    property Item control

    width: theme.checkboxSize - 4
    height: theme.checkboxSize - 4

    radius: Math.max(theme.frameRadius - 1, 0)

    readonly property int animationDuration: 150

    gradient: Gradient {
        GradientStop {
            position: 0.0
            color: root.topColor
        }
        GradientStop {
            position: 1.0
            color: root.bottomColor
        }
    }

    property color bottomColor: {
        return theme.getCheckBoxBottomColor(control.hovered, control.down, control.checked || ((typeof control.checkState !== "undefined") && control.checkState == Qt.Checked))
    }

    property color topColor: {
        return theme.getCheckBoxTopColor(control.hovered, control.down, control.checked || ((typeof control.checkState !== "undefined") && control.checkState == Qt.Checked))
    }

    border {
        color: theme.getCheckBoxOutlineColor(control.hovered, control.down, control.checked || ((typeof control.checkState !== "undefined") && control.checkState == Qt.Checked))
    }

    Item {
        visible: (typeof control.checkState !== "undefined") && control.checkState == Qt.Checked
        opacity: visible
        rotation: -45
        anchors.fill: parent
        Rectangle {
            color: theme.highlightTextColor
            x: 5
            y: 4
            width: 3
            height: 6
            radius: 4
        }
        Rectangle {
            color: theme.highlightTextColor
            x: 5
            y: 8
            width: 10
            height: 3
            radius: 7
        }

        Behavior on opacity { PropertyAnimation { duration: root.animationDuration } }
    }
}
