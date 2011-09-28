import QtQuick 1.0
import org.kde.analitza 1.0
import "widgets"

KAlgebraPage
{
// 	id: bg
	width: 640; height: 500
// 	color: "red"
	anchors.fill: parent
	
	Analitza {
		id: a
	}
	
	function doOp(str)
	{
		console.log(str);
		var start=input.selectionStart, end = input.selectionStart;
		var text = input.text;
		
		console.log(":: " + start + " " + text.substr(0,start));
// 		input.text += str;
		
		input.text = text.substr(0,start)+str+text.substr(end,text.size)
		input.select(start+1,start+1);
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
			if(!res)
				res = a.error
			
			itemModel.append({ result: text + " = " + res });
			input.selectAll();
			
			console.log(":) "+itemModel.count);
// 			view.positionViewAtIndex(itemModel.count, ListView.End)
			view.currentIndex = itemModel.count-1
		}
		
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.top: parent.top
	}
	
	Row {
		id: inputrow
		
		Button {
			id: executeButton
			text: "Exec"
		}
		
		anchors.top: input.bottom
		anchors.right: parent.right
	}
	
	SimpleListView {
		model: itemModel
// 		delegate: ConsoleDelegate {}
// 		itemDelegate: Row { Text { text: result } }
		id: view
		
		height: 200
		
		role: "result"
		
		anchors.top: inputrow.bottom
		anchors.bottom: parent.bottom
		width: parent.width
	}
}