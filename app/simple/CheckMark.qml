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

Item {
    width: $(64)
    height: $(64)
    property real shorterSide: width < height ? width : height
    property color color: Qt.tint("light green", "green")
    Rectangle {
        rotation: 45
        x: shorterSide * 0.025
        y: shorterSide * 0.45
        width: shorterSide * 0.45
        height: shorterSide * 0.25
        color: parent.color
    }
    Rectangle {
        rotation: -45
        x: shorterSide * 0.19
        y: shorterSide * 0.39
        width: shorterSide * 0.845
        height: shorterSide * 0.25
        color: parent.color
    }
}
