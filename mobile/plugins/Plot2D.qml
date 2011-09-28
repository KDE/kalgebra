import QtQuick 1.0
import org.kde.analitza 1.0
import "widgets"

KAlgebraPage
{
	function addFunc() { app.addFunction(input.text) }
	
	Column {
		id: controls
		anchors.fill: parent
		
		Rectangle {
			width: parent.width
			height: 200
			color: 'white'
			
			Graph2D {
				id: view
				anchors.fill: parent
				
				squares: true
				model: app.functionsModel()
			}
		}
		
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
			model: app.functionsModel()
			
			onCurrentIndexChanged: view.currentFunction=currentIndex-1
			
			role: "expression"
			
// 			anchors.bottom: parent.bottom
			
			width: parent.width
			height: 200
		}
	}
	
}