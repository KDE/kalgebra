import QtQuick 1.1

Rectangle {
	property Flickable flickableItem: null
	anchors.right: flickableItem.right
	visible: flickableItem!=null
	width: 10
	height: flickableItem.height/(flickableItem.contentHeight/flickableItem.height)
	y: (flickableItem.contentY/flickableItem.contentHeight)*flickableItem.height
	
	SystemPalette { id: palette }
	color: palette.text
}