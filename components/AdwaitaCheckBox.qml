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
                    color: "black"
                    x: $(5)
                    y: $(4)
                    width: $(3)
                    height: $(6)
                    radius: $(4)
                }
                Rectangle {
                    color: "black"
                    x: $(5)
                    y: $(8)
                    width: $(12)
                    height: $(3)
                    radius: $(4)
                }
            }
        }
        label: Text {
            font.pixelSize: $(12)
            text: control.text
        }
        spacing: $(8)
    }
}

