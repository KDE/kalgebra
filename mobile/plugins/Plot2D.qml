import QtQuick 1.0
import org.kde.analitza 1.0
import "widgets"

Rectangle
{
	width: 500
	height: 500
	
	Graph2D {
		id: view
		height:parent.height-(input.height+listview.height)
		width: parent.width
		squares: true
		model: app.functionsModel()
		
		anchors.top: parent.top;
	}
	
	function addFunc() { app.addFunction(input.text) }
	
	ListView {
		id: listview
		model: app.functionsModel()
		delegate: Row { Text { text: ">>> "+display+" <<<"; font.bold: selection } Text { text: selection } }
		
		height: 100
		anchors.top: input.bottom
		anchors.left: parent.left
		anchors.right: parent.right
	}
	
	ExpressionInput {
		id: input
		text: "sin x"
		
		Keys.onReturnPressed: addFunc()
		
		anchors.top: view.bottom
		anchors.left: parent.left
		anchors.right: parent.right
// 		anchors.bottom: parent.bottom
	}
	
	Button {
		id: exec
		text: "Add"
		
		height: 80; width: 80
		anchors.bottom: parent.bottom
		anchors.right: parent.right
		
		onClicked: addFunc()
	}
	
// 	Rectangle { color: "red"; anchors.fill: parent}
}