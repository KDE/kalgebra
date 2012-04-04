import QtQuick 1.1

Rectangle {
	radius: 3
    height: buttonText.height+30
    width: buttonText.width+30
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
