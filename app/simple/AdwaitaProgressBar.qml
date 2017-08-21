/*
 * Fedora Media Writer
 * Copyright (C) 2016 Martin Bříza <mbriza@redhat.com>
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

import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2

Rectangle {
    id: root
    property color progressColor: palette.highlight
    property color backgroundColor: Qt.darker(palette.button, 1.2)
    property real maximumValue: 1.0
    property real minimumValue: 0.0
    property real value: 0

    width: $(100)
    height: $(6)

    border {
        color: Qt.darker(palette.button, 1.7)
        width: 1
    }
    radius: $(3)
    clip: true
    gradient: Gradient {
        GradientStop { position: 0.0; color: Qt.lighter(root.backgroundColor, 1.05) }
        GradientStop { position: 1.0; color: Qt.darker(root.backgroundColor, 1.05) }
    }

    Rectangle {
        clip: true
        y: 0.5
        x: 0.5
        height: $(6) - 1
        width: (root.value - root.minimumValue) / (root.maximumValue - root.minimumValue) * (parent.width - 1);
        border {
            color: Qt.darker(palette.button)
            width: 1
        }
        radius: $(3)
        gradient: Gradient {
            GradientStop { position: 0.0; color: Qt.lighter(root.progressColor, 1.05) }
            GradientStop { position: 0.9; color: root.progressColor }
            GradientStop { position: 1.0; color: Qt.darker(root.progressColor) }
        }
    }

    AdwaitaBusyIndicator {
        anchors.fill: parent
        visible: isNaN(root.value)
        progressColor: root.progressColor
    }
}

