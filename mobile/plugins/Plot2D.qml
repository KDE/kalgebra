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
		model: app.functionsModel()
		
		anchors.top: parent.top;
	}
	
	function addFunc() {
		app.addFunction(input.text)
		console.log("lalala "+input.text)
	}
	
	ExpressionInput {
		id: input
		text: "sin x"
		
		Keys.onReturnPressed: {
			addFunc()
		}
		
		anchors.top: view.bottom
// 		anchors.bottom: parent.bottom
	}
	
	Button {
		id: exec
		text: "Add"
		
		height: 80; width: 80
		anchors.bottom: parent.bottom
		anchors.right: parent.right
		
		onClicked: {
			addFunc()
		}
	}
	
// 	Rectangle { color: "red"; anchors.fill: parent}
}