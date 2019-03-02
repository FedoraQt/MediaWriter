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

RadioButton {
    id: root
    style: RadioButtonStyle {
        indicator: AdwaitaRectangle {
            implicitWidth: root.height
            implicitHeight: root.height
            radius: width / 2 + 1
            Rectangle {
                anchors.centerIn: parent
                width: parent.width / 3
                height: parent.width / 3
                radius: parent.radius / 3
                color: palette.text
                visible: control.checked
            }
        }
        label: Text {
            id: text
            font.pointSize: $$(9)
            text: control.text
            color: palette.windowText
        }
        spacing: $(6)
    }
}

