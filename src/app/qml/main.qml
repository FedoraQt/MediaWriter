import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Window 2.12
import QtQuick.Layouts 1.12
import QtQml 2.12

ApplicationWindow {
    id: mainWindow
    visible: true
    minimumWidth: 800
    minimumHeight: 480
    title: "Welcome"
    color: "white"
    

    ColumnLayout{
        id: mainColumn
        anchors.fill: parent

        RowLayout{
            id: obrazekAtext
            Layout.alignment: Qt.AlignHCenter

            Rectangle{
                id: obrazek
                border.color: "black"
                border.width: 2
                width: 300
                height: 200
                
                
                Text{
                    anchors.centerIn: parent
                    text: "Sem prijde obrazek"
                }
            }
        }

        RowLayout{
            Layout.alignment: Qt.AlignHCenter
            Label{
                id: mainLabel
                
                text: "Select Image Source"
                font.pixelSize: 22
            }
        }

        ColumnLayout{
            Layout.alignment: Qt.AlignHCenter
            RadioButton{
                checked: true
                text: "Download automatically"
                font.pixelSize: 16
            }
        
            RadioButton{
                text: "Select .iso file"
                font.pixelSize: 16
            }
        }

        RowLayout{
            Layout.margins: 30
            Layout.leftMargin: 40
            Layout.rightMargin: 40
            
            Button{
                id: aboutButton
                text: qsTr("About")
                
                contentItem: Text{
                    text: parent.text
                    font.pixelSize: 18
                    color: aboutButton.down ? "red" : "black"
                    verticalAlignment: Text.AlignHCenter
                    horizontalAlignment: Text.AlignHCenter
                }
                
                background: Rectangle{
                    radius: 10
                    implicitWidth: 100
                    implicitHeight: 40
                    border.color: "#cdd0ce"
                    border.width: 1
                }
            }
        
            Item {
                Layout.fillWidth: true
            }
            
            Button{
                id: nextButton
                text: qsTr("Next")

                contentItem: Text{
                    text: parent.text
                    font.pixelSize: 18
                    color: nextButton.down ? "red" : "black"
                    verticalAlignment: Text.AlignHCenter
                    horizontalAlignment: Text.AlignHCenter
                }
                
                background: Rectangle{
                    radius: 10
                    implicitWidth: 100
                    implicitHeight: 40
                    border.color: "#cdd0ce"
                    border.width: 1
                }
            }
        }
    }
}

