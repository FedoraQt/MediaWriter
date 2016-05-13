import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2

BusyIndicator {
    id: root
    width: $(148)
    height: $(6)
    property color progressColor: "#54aada"
    property color backgroundColor: "#c3c3c3"

    onRunningChanged: {
        if (running) {
            flyingAnimation.stop()
            flyingBar.x = 0
        }
        else {
            flyingAnimation.start()
        }
    }

    Rectangle {
        implicitWidth: root.width
        width: root.width
        height: root.height
        border {
            color: "#777777"
            width: $(1)
        }
        radius: $(3)
        clip: false
        gradient: Gradient {
            GradientStop { position: 0.0; color: Qt.lighter(root.backgroundColor, 1.05) }
            GradientStop { position: 1.0; color: Qt.darker(root.backgroundColor, 1.05) }
        }
        Rectangle {
            id: flyingBar
            width: $(32)
            height: root.height - $(2)
            y: $(1)
            radius: $(3)
            border {
                color: "#777777"
                width: $(1)
            }
            opacity: root.running ? 1.0 : 0.0
            Behavior on opacity { NumberAnimation { } }
            gradient: Gradient {
                GradientStop { position: 0.0; color: Qt.lighter(root.progressColor, 1.05) }
                GradientStop { position: 0.9; color: root.progressColor }
                GradientStop { position: 1.0; color: Qt.darker(root.progressColor) }
            }
            SequentialAnimation {
                id: flyingAnimation
                running: root.running
                loops: Animation.Infinite
                NumberAnimation {
                    duration: 1000
                    target: flyingBar
                    easing.type: Easing.InOutCubic
                    property: "x"
                    from: $(1)
                    to: root.width - flyingBar.width
                }
                NumberAnimation {
                    duration: 1000
                    target: flyingBar
                    easing.type: Easing.InOutCubic
                    property: "x"
                    from: root.width - flyingBar.width
                    to: $(1)
                }
            }
        }
    }

    // this style is completely useless, let's just implement it outside
    style: BusyIndicatorStyle {
        indicator: Item {
        }
    }
}
