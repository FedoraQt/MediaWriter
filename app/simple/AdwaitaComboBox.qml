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
import QtQuick.Window 2.0

AdwaitaRectangle {
    id: control
    z: 3
    implicitWidth: $(128)
    property alias model: options.model
    property string textRole: ""
    property alias currentIndex: options.currentIndex
    property string currentText: options.currentItem ? options.currentItem.text : ""
    property string placeholderText: ""
    property alias count: options.count

    property bool hovered: mouse.containsMouse
    property bool pressed: isOpen

    property bool isOpen: false
    activeFocusOnTab: true

    signal activated(int index)

    onIsOpenChanged: {
        container.update()
    }

    enabled: options.count > 0

    QtObject {
        id: container
        property var item: (typeof(dialog) !== 'undefined') ? dialogContainer : (typeof(mainWindow) !== 'undefined') ? mainWindowContainer : null
        property real x: control.mapFromItem(item, 0, 0).x
        property real y: control.mapFromItem(item, 0, 0).y
        property real height: item.height
        property real width: item.width
        function update() {
            container.x = control.mapFromItem(item, 0, 0).x
            container.y = control.mapFromItem(item, 0, 0).y
        }
    }

    Arrow {
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.rightMargin: $(12)
        scale: $(1.25)
        rotation: 90
    }

    Text {
        x: $(9)
        anchors.verticalCenter: parent.verticalCenter
        text: count > 0 && options.currentItem ? options.currentItem.text : placeholderText
        color: enabled ? palette.text : disabledPalette.text
        font.pointSize: $$(9)
    }

    Keys.onUpPressed: {
        if (currentIndex > 0)
            currentIndex--
    }
    Keys.onDownPressed: {
        if (currentIndex < count - 1)
            currentIndex++
    }
    Keys.onSpacePressed: mouse.action()

    MouseArea {
        id: mouse
        hoverEnabled: true
        enabled: !isOpen
        anchors.fill: parent
        function action() {
            if (count > 0) {
                isOpen = true
                options.focus = true
            }
        }
        onClicked: action()
    }

    // area capturing clicks around the open dropdown... a bit hacky
    MouseArea {
        enabled: isOpen
        width: Screen.width * 2
        height: Screen.height * 2
        x: -Screen.width
        y: -Screen.height
        onClicked: isOpen = false
    }

    Rectangle {
        anchors.fill: options
        color: palette.base
        border.color: Qt.darker(palette.button, 1.5)
        border.width: 1
        visible: options.visible
    }

    ListView {
        property real potentialHeight: Math.min(count * control.height + headerItem.height + footerItem.height, container.height)
        property real potentialY: -currentIndex * control.height - headerItem.height
        property real potentialOverflow: potentialY + potentialHeight > container.y + container.height ? potentialY + potentialHeight - container.y - container.height : 0
        height: control.isOpen ? potentialHeight : 0
        y: (potentialY - potentialOverflow) < container.y ? container.y : (potentialY - potentialOverflow)
        id: options
        visible: isOpen
        width: parent.width
        clip: true

        header: Item { height: 6; width: 1 }
        footer: Item { height: 6; width: 1 }

        // an item contains mouse - used to determine if the currently selected item should be highlighted
        property bool itemContainsMouse: false

        delegate: Rectangle {
            color: (ListView.isCurrentItem && !options.itemContainsMouse) || itemMouse.containsMouse ? palette.highlight : "transparent"
            property string text: label.text
            height: control.height
            width: control.width
            Text {
                color: ListView.isCurrentItem ? palette.highlightedText : palette.text
                id: label
                x: $(9)
                anchors.verticalCenter: parent.verticalCenter
                text: textRole ? model[textRole] : modelData
                font.pointSize: $$(9)
            }
            Keys.onSpacePressed: itemMouse.action()
            MouseArea {
                id: itemMouse
                hoverEnabled: true
                anchors.fill: parent
                function action() {
                    options.currentIndex = index
                    control.isOpen = false
                    activated(index)
                }

                onClicked: {
                    action()
                }
                onContainsMouseChanged: {
                    if (containsMouse)
                        options.itemContainsMouse = true
                    else
                        options.itemContainsMouse = false
                }
            }
        }
    }
}

