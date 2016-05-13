import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2

Rectangle {
    id: root
    property color progressColor: "#54aada"
    property color backgroundColor: "#c3c3c3"
    property real maximumValue: 1.0
    property real minimumValue: 0.0
    property real value: 0

    width: $(100)
    height: $(6)

    border {
        color: "#777777"
        width: $(1)
    }
    radius: $(3)
    clip: true
    gradient: Gradient {
        GradientStop { position: 0.0; color: Qt.lighter(root.backgroundColor, 1.05) }
        GradientStop { position: 1.0; color: Qt.darker(root.backgroundColor, 1.05) }
    }

    Rectangle {
        clip: true
        y: $(0.5)
        x: $(0.5)
        height: $(5)
        width: (root.value - root.minimumValue) / (root.maximumValue - root.minimumValue) * (parent.width - 1);
        border {
            color: "#777777"
            width: $(1)
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
    }
}

