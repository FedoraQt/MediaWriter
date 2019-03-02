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

import "../simple"

AdwaitaButton {
    implicitWidth: arrow.width + text.width + $(28)
    Item {
        id: arrow
        anchors.left: parent.left
        anchors.leftMargin: $(12)
        anchors.verticalCenter: parent.verticalCenter
        rotation: -45
        transformOrigin: Item.Center
        width: $(10)
        height: $(10)
        Rectangle {
            x: $(1.5)
            y: $(1.5)
            width: $(2)
            height: $(9)
            radius: $(2)
            color: mixColors(palette.button, palette.text, 0.5)
        }
        Rectangle {
            y: $(1.5)
            x: $(1.5)
            width: $(9)
            height: $(2)
            radius: $(2)
            color: mixColors(palette.button, palette.text, 0.5)
        }
    }
    Text {
        id: text
        text: qsTr("Back")
        color: palette.text
        font.pointSize: $$(9)
        anchors {
            left: arrow.left
            leftMargin: $(16)
            verticalCenter: parent.verticalCenter
        }
    }
}
