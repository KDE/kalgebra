import QtQuick 2.0
import org.kde.analitza 1.0
import widgets 1.0

KAlgebraPage
{
	anchors.margins: 0
	
	Row {
		id: controls
		spacing: 10
		
		width: parent.width
		ExpressionInput {
			id: input
			width: parent.width-exec.width
			anchors.verticalCenter: exec.verticalCenter
			text: "sin x"
			focus: true
			Component.onCompleted: selectAll()
			
			Keys.onReturnPressed: view.addFunction(input.text, app.variables)
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
	
	
	Rectangle {
		width: parent.width
		anchors {
			fill: parent
			topMargin: controls.height
		}
		height: 200
		color: 'white'
	
		Graph2D {
			id: view
			anchors.fill: parent
			model: app.functionsModel()
		}
	}
}
