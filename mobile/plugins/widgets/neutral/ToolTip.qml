import QtQuick 2.0

Rectangle {
	id: tooltip
	property string text
	Text {
		text: tooltip.text
		
		anchors.fill: parent
	}
	
	SystemPalette { id: palette }
	color: palette.light
}