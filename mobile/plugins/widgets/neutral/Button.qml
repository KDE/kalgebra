import QtQuick 1.1

Rectangle {
    height: buttonText.height+10
    width: buttonText.width+10
    property alias text: buttonText.text
    color: buttonArea.containsMouse ? "red" : "blue"
    signal clicked
    
    Text {
        id: buttonText
        anchors.centerIn: parent
    }
    
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        id: buttonArea
        onClicked: parent.clicked()
    }
}
