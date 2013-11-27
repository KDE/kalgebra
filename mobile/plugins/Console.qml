import QtQuick 2.0
import org.kde.analitza 1.0
import widgets 1.0

KAlgebraPage
{
	ListModel { id: itemModel }
	
	ExpressionInput {
		id: input
		focus: true
		
		Analitza {
			id: a
			variables: app.variables
		}
		
		Keys.onReturnPressed: {
			var res = a.execute(text)
			
			var toadd = ""
			if(!a.isCorrect)
				toadd = "Error: " + (res ? res : a.errors)
			else
				toadd = text + " = " + res.expression
			
			itemModel.insert(0, { result: toadd, resultsInput: text })
			input.selectAll()
			view.currentIndex = 0
		}
		
		width: parent.width
		anchors.top: parent.top
	}
	
	SimpleListView {
		id: view
		model: itemModel
		role: "result"
		title: "Log"
		
		anchors {
			top: input.bottom
			bottom: parent.bottom
			left: parent.left
			right: parent.right
		}
	}
}
