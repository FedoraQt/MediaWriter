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
import QtQuick.Shapes 
import QtQuick.Templates as T
import org.fedoraproject.AdwaitaTheme 
import "private" as Private

T.ComboBox {
    id: control

    implicitWidth: Math.max(background ? background.implicitWidth : 0,
                            contentItem.implicitWidth) + leftPadding + rightPadding + spacing + indicator.implicitWidth + rightPadding
    implicitHeight: Math.max(background ? background.implicitHeight : 0,
                             contentItem.implicitHeight + bottomPadding + topPadding)

    spacing: theme.menuItemSpacing
    leftPadding: theme.comboBoxMarginWidth + theme.frameWidth
    rightPadding: theme.comboBoxMarginWidth + theme.frameWidth
    bottomPadding: theme.comboBoxMarginHeight + theme.frameWidth
    topPadding: theme.comboBoxMarginHeight + theme.frameWidth

    hoverEnabled: true

    delegate: MenuItem {
        width: parent.width
        text: control.textRole ? (Array.isArray(control.model) ? modelData[control.textRole] : model[control.textRole]) : modelData
        font.weight: control.currentIndex === index ? Font.DemiBold : Font.Normal
        highlighted: control.highlightedIndex == index
        hoverEnabled: control.hoverEnabled
    }

    indicator: Shape {
        implicitHeight: 10
        implicitWidth: 10
        x: control.mirrored ? control.leftPadding - 2 : control.width - width - control.rightPadding + 2
        y: control.topPadding + control.availableHeight / 2

        ShapePath {
            fillColor: theme.textColor
            strokeColor: theme.textColor
            capStyle: ShapePath.FlatCap
            joinStyle: ShapePath.MiterJoin

            startX: -4
            startY: -2
            PathLine { x: 0; y: 2 }
            PathLine { x: 4; y: -2 }
        }
    }

    contentItem: T.TextField {
        anchors {
            fill: parent
            leftMargin: control.leftPadding
            rightMargin: control.rightPadding + indicator.width + control.spacing
            bottomMargin: control.bottomPadding
            topMargin: control.topPadding
        }
        implicitWidth: contentWidth
        text: control.editable ? control.editText : control.displayText

        enabled: control.editable
        autoScroll: control.editable
        readOnly: control.down
        inputMethodHints: control.inputMethodHints
        validator: control.validator

        font: control.font
        color: control.enabled ? theme.textColor : theme.disabledTextColor
        selectionColor: theme.highlightColor
        selectedTextColor: theme.highlightTextColor
        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle {
        id: rect
        anchors.fill: parent
        implicitHeight: theme.comboBoxMinimumHeight
        implicitWidth: theme.comboBoxMinimumWidth
        radius: theme.frameRadius
        visible: control.flat ? control.pressed || control.checked : true

        readonly property int animationDuration: 150

        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: control.editable ? control.enabled ? theme.baseColor : theme.windowColor : rect.topColor
                Behavior on color { ColorAnimation { duration: rect.animationDuration } }
            }
            GradientStop {
                position: 1.0
                color: control.editable ? control.enabled ? theme.baseColor :theme.windowColor : rect.bottomColor
                Behavior on color { ColorAnimation { duration: rect.animationDuration } }
            }
        }

        property color bottomColor: {
            return theme.getButtonBottomColor(false, false, control.hovered, control.pressed || control.checked)
        }

        property color topColor: {
            return theme.getButtonTopColor(false, false, control.hovered, control.pressed || control.checked)
        }

        border {
            color: control.editable ? control.focus ? theme.highlightColor : theme.getButtonOutlineColor(false, false, control.hovered, false) :
                                      theme.getButtonOutlineColor(false, false, control.hovered, control.pressed || control.checked)
        }
    }

    popup: T.Popup {
        x: control.mirrored ? control.width - width : 0
        y: control.height
        width: control.width
        height: contentItem.implicitHeight
        topMargin: 6
        bottomMargin: 6

        contentItem: ListView {
            id: listView
            clip: true
            implicitHeight: contentHeight
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex
            highlightRangeMode: ListView.ApplyRange
            highlightMoveDuration: 0

            T.ScrollBar.vertical: ScrollBar { }
        }

        background: Rectangle {
            anchors {
                fill: parent
                margins: 1
            }
            color: theme.baseColor
            border.color: theme.getButtonOutlineColor(false, false, false, false)
            radius: theme.frameRadius
        }
    }
}
