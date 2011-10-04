import QtQuick 1.0
import org.kde.analitza 1.0
import "widgets"

KAlgebraPage
{
	Analitza {
		id: a
	}
	
	function doSelectAll()
	{
		input.selectAll()
	}
	
	ListModel { id: itemModel }
	
	ExpressionInput {
		id: input
		focus: true
		
		Keys.onReturnPressed: {
			var res = a.execute(text)
			
			if(!a.isCorrect) {
				if(res)
					itemModel.append({ result: "Error: " + res })
				else
					itemModel.append({ result: "Error: " + a.errors })
			} else
				itemModel.append({ result: text + " = " + res.expression });
			
			input.selectAll();
			
			view.currentIndex = itemModel.count-1
		}
		
		width: parent.width
		anchors.top: parent.top
	}
	
	SimpleListView {
		model: itemModel
		id: view
		
		height: 200
		
		role: "result"
		
		anchors.top: input.bottom
		anchors.bottom: parent.bottom
		width: parent.width
	}
}