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
import QtQuick.Controls.Private 1.0

ComboBox {
    id: root
    implicitWidth: $(128)
    implicitHeight: $(32)
    style: ComboBoxStyle {
        background: AdwaitaRectangle {
            width: control.width
            Arrow {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.rightMargin: $(12)
                scale: $(1.2)
                rotation: 90
            }
        }
        label: Text {
            width: control.width
            x: $(6)
            font.pointSize: $(9)
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
            text: control.count > 0 ? control.currentText : ""
            color: root.enabled ? palette.buttonText : disabledPalette.buttonText
        }
    }
}

