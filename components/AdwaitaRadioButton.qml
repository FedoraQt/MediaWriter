import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2

RadioButton {
    style: RadioButtonStyle {
        indicator: AdwaitaRectangle {
            implicitWidth: $(15)
            implicitHeight: $(15)
            radius: width / 2 + 1
            Rectangle {
                anchors.centerIn: parent
                width: parent.width / 3
                height: parent.width / 3
                radius: parent.radius / 3
                color: "black"
                visible: control.checked
            }
        }
        label: Text {
            font.pixelSize: $(12)
            text: control.text
        }
        spacing: $(6)
    }
}

