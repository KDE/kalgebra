import QtQuick 1.0
import org.kde.analitza 1.0
import "widgets"

KAlgebraPage
{
	function addFunc() { app.addFunction(input.text) }
	
	height: 100
	width: 100
	
	Rectangle {
		width: parent.width
		anchors.top: parent.top
		anchors.bottom: controls.top
		color: 'white'
		
		Graph2D {
			id: view
			anchors.fill: parent
			
			squares: true
			model: app.functionsModel()
		}
	}
	
	Column {
		id: controls
		anchors.bottom: parent.bottom
		
		Row {
			width: parent.width
			ExpressionInput {
				id: input
				text: "sin x"
				
				Keys.onReturnPressed: addFunc()
			}
			
			Button {
				id: exec
				text: "Add"
				
				onClicked: addFunc()
			}
		}
		
		SimpleListView {
			id: listview
			model: app.functionsModel()
// 			itemDelegate: Text { text: display + " " + expression }
// 				highlight: Rectangle { color: "lightsteelblue"; radius: 5 }
			
			onCurrentIndexChanged: view.currentFunction=currentIndex-1
			
			role: "expression"
			
			width: view.width
			height: 100
		}
	}
	
}