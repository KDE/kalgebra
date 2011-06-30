import QtQuick 1.0
import org.kde.analitza 1.0
import "widgets"

Rectangle
{
	width: 500
	height: 500
	
	Graph2D {
		id: view
		height:parent.height-input.height
		width: parent.width
		squares: true
		model: model
		
		anchors.top: parent.top;
	}
	
	FunctionsModel {
		id: model
	}
	
	ExpressionInput {
		id: input
		text: "sin x"
		
		Keys.onReturnPressed: {
			
		}
		
		anchors.top: view.bottom
// 		anchors.bottom: parent.bottom
	}
	
// 	Rectangle { color: "red"; anchors.fill: parent}
}