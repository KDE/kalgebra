import QtQuick 1.0
import org.kde.analitza 1.0
import "widgets"

KAlgebraPage
{
	function addFunc() { app.addFunction(input.text) }
	
	Column {
		id: controls
		anchors.fill: parent
		
		Graph2D {
			width: parent.width
			height: 200
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
			
			role: "expression"
			
// 			anchors.bottom: parent.bottom
			
			width: parent.width
			height: 200
		}
	}
	
}