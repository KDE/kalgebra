import QtQuick 1.1

Item {
	property alias text: display.text
	signal clicked
	width: display.width+15
	height: display.height+15
	property alias implicitWidth: display.implicitWidth
	property alias implicitHeight: display.implicitHeight
	
	SystemPalette { id: palette }
	
	Rectangle {
		radius: 5
		opacity: 0.3
		anchors.fill: parent
		anchors.margins: 2
		color: palette.button
		border.width: buttonArea.containsMouse ? 5 : 1
		border.color: palette.buttonText
	}
	
	Text { id: display; anchors.centerIn: parent; color: palette.buttonText }
	
	MouseArea {
		anchors.fill: parent
		hoverEnabled: true
		id: buttonArea
		onClicked: parent.clicked()
	}
}
