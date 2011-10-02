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
			height: parent.height-input.height
		}
		
		Row {
			width: parent.width
			ExpressionInput {
				id: input
				width: parent.width-exec.width
				text: "sin x"
				focus: true
				Component.onCompleted: selectAll()
				
				Keys.onReturnPressed: addFunc()
			}
			
			Button {
				id: exec
				text: "Clear"
				
				onClicked: app.functionsModel().clear()
			}
		}
		
/*		SimpleListView {
			id: view
			model: app.functionsModel()
			
			role: "expression"
			
			width: parent.width
			height: 200
		}*/
	}
}
