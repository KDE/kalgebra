import QtQuick 1.0
import org.kde.analitza 1.0
import "widgets"

KAlgebraPage
{
	id: bg
	width: 100
	height: 100
	
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
			resultsModel.append( { element: i +" = "+ a.executeFunc(tmp, args) } );
		}
		
		a.removeVariable(tmp);
	}
	
	Grid {
		id: ins
		columns: 2
		
		Label {text: "Input: " } ExpressionInput { id: input; text: "x->sin x"}
		Label {text: "From:" }   RealInput { id: from; value: 0 }
		Label {text: "To:" }     RealInput { id: to; value: 10 }
		Label {text: "Step:" }   RealInput { id: step; value: 1 }
		
		Button {
			text: "Go!"
			
			onClicked: calculateTable()
		}
	}
	
	Label { text: "Results:"; id: res; anchors.top: ins.bottom }
	
	SimpleListView {
		id: view
		anchors.bottom: parent.bottom
		anchors.top: res.bottom
		
		model: resultsModel
		width: bg.width
		role: "element"
	}
}
