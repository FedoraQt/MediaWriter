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
import org.fedoraproject.AdwaitaTheme

Item {
    id: root

    // TODO add a focus rect

    property bool flat: false
    property bool highlighted: false
    property bool destructiveAction: false

    Rectangle {
        id: rect
        anchors.fill: parent
        radius: theme.frameRadius
        visible: flat ? control.pressed || control.checked : true

        readonly property int animationDuration: 150

        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: rect.topColor
                Behavior on color { ColorAnimation { duration: rect.animationDuration } }
            }
            GradientStop {
                position: 1.0
                color: rect.bottomColor
                Behavior on color { ColorAnimation { duration: rect.animationDuration } }
            }
        }

        property color bottomColor: {
            return theme.getButtonBottomColor(root.highlighted, root.destructiveAction, control.hovered, control.pressed || control.checked)
        }

        property color topColor: {
            return theme.getButtonTopColor(root.highlighted, root.destructiveAction, control.hovered, control.pressed || control.checked)
        }

        border {
            color: theme.getButtonOutlineColor(root.highlighted, root.destructiveAction, control.hovered, control.pressed || control.checked)
        }
    }
}
