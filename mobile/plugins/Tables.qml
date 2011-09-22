import QtQuick 1.0
import QtDesktop 0.1
import org.kde.analitza 1.0
import "widgets"

KAlgebraPage
{
	id: bg
	
	ListModel { id: resultsModel }
	Analitza { id: a }
	
	function calculateTable() {
		resultsModel.clear();
		
		var tmp = a.unusedVariableName();
		var ret = a.execute(tmp+":="+input.text, true)
		var ffrom=from.value, fto=to.value, fstep=step.value;
// 		console.log("chancho (" + ffrom + ", " + fto + ") " + ret);
		
		for (var i=ffrom; i<=fto; i+=fstep) {
			var args = new Array();
			args[0]=i;
// 			console.log("!!! " + i); 
			resultsModel.append( { value: i, result: a.executeFunc(tmp, args) } );
		}
		
		a.removeVariable(tmp);
	}
	
	Grid {
		id: ins
		columns: 2
		
		Text {text: "Input: " } ExpressionInput { id: input; text: "x->sin x"}
		Text {text: "From:" }   RealInput { id: from; value: 0 }
		Text {text: "To:" }     RealInput { id: to; value: 10 }
		Text {text: "Step:" }   RealInput { id: step; value: 1 }
		
		Button {
			text: "Go!"
			
			onClicked: calculateTable()
		}
	}
	
	Text { text: "Results:"; id: res; anchors.top: ins.bottom }
	
	TableView {
		id: view
		anchors.bottom: parent.bottom
		anchors.top: res.bottom
		
		model: resultsModel
		width: bg.width
// 		delegate: Text { text: value+" -> "+result}

		TableColumn {
			property: "value"
			caption: "Value"
			width: 50
		}
		
		TableColumn {
			property: "result"
			caption: "Image"
		}
	}
}
