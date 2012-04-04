import QtQuick 1.0
import org.kde.analitza 1.0
import widgets 1.0

KAlgebraPage
{
	function addFunc() { app.addFunction(input.text) }
	
	anchors.margins: 0
	
	Column {
		id: controls
		anchors.fill: parent
		
		Row {
			spacing: 10
			
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
				width: 100
				
				onClicked: {
					app.functionsModel().clear()
					view.resetViewport()
					input.focus = true
				}
			}
		}
		
		Graph2D {
			id: view
			width: parent.width
			height: parent.height-input.height
		}
	}
}
