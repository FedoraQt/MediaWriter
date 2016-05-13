import QtQuick 2.3

Item {
    id: root
    implicitHeight: $(36)
    implicitWidth: $(36)

    property alias radius: rect.radius
    property color color: palette.button


    Rectangle {
        id: rect
        anchors.fill: parent
        radius: $(3)

        readonly property int animationDuration: 150
        color: control.enabled ? root.color : "light gray"
        Behavior on color { ColorAnimation { duration: rect.animationDuration } }

        border {
            width: 1
            color: Qt.colorEqual(root.color, "transparent") ? "#212121" : control.enabled ? "#777777" : "#c2c2c2"
        }

        Rectangle {
            id: overlay
            radius: parent.radius - $(1)
            anchors.margins: $(0.5)
            anchors.fill: parent
            //gradient: control.enabled ? !(control.pressed || control.checked) ? !control.hovered ? regularGradient: hoveredGradient : downGradient : disabledGradient
            gradient: Gradient {
                GradientStop {
                    position: 0.0
                    color: overlay.topColor
                    Behavior on color { ColorAnimation { duration: 100 } }
                }
                GradientStop {
                    position: 1.0
                    color: overlay.bottomColor
                    Behavior on color { ColorAnimation { duration: 100 } }
                }
            }

            property color topColor: control.enabled ? !(control.pressed || control.checked) ? !control.hovered ?  "#14ffffff" : "#14ffffff" : "#1e000000" : "transparent"
            property color bottomColor: control.enabled ? !(control.pressed || control.checked) ? !control.hovered ? "#14000000" : "#05ffffff" : "#14000000" : "transparent"
        }
    }

    SystemPalette {
        id: palette
    }
}
