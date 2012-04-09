import QtQuick 1.1

Item {
	property alias text: display.text
	signal clicked
	width: display.width+30
	height: display.height+30
	
	Text { id: display; anchors.centerIn: parent }
	
	Rectangle {
		radius: 5
		opacity: 0.3
		anchors.fill: parent
		color: buttonArea.containsMouse ? "red" : "blue"
	}
	
	MouseArea {
		anchors.fill: parent
		hoverEnabled: true
		id: buttonArea
		onClicked: parent.clicked()
	}
}
