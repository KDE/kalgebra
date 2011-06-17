import QtQuick 1.0
import "widgets"

Rectangle
{
	width: 200
	height: 300
	
	Column {
		anchors.fill: parent
		
		Rectangle { //plotting area
			color: "red"
			height:200
			width: 100
			anchors.horizontalCenter: parent.horizontalCenter
		}
		
		ExpressionInput {}
	}
}