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

CheckBox {
    style: CheckBoxStyle {
        indicator: AdwaitaRectangle {
            implicitWidth: $(15)
            implicitHeight: $(15)
            Item {
                visible: control.checked
                rotation: -45
                anchors.fill: parent
                Rectangle {
                    color: palette.text
                    x: $(5)
                    y: $(4)
                    width: $(3)
                    height: $(6)
                    radius: $(4)
                }
                Rectangle {
                    color: palette.text
                    x: $(5)
                    y: $(8)
                    width: $(12)
                    height: $(3)
                    radius: $(4)
                }
            }
        }
        label: Text {
            font.pointSize: $$(9)
            text: control.text
            color: control.enabled ? palette.windowText : disabledPalette.windowText
        }
        spacing: $(8)
    }
}

