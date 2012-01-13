import QtQuick 1.0
import org.kde.analitza 1.0
import "widgets"

KAlgebraPage
{
	ListModel { id: resultsModel }
	Analitza { id: a }
	
	function calculateTable() {
		resultsModel.clear();
		
		var tmp = a.unusedVariableName()
		var ret = a.insertVariable(tmp, a.dependenciesToLambda(input.text))
		var ffrom=from.value, fto=to.value, fstep=step.value;
// 		console.log("chancho (" + ffrom + ", " + fto + ") " + ret);
		
		if(!a.isCorrect) {
			if(ret)
				resultsModel.append( { element: "Errors: "+ret } );
			else
				resultsModel.append( { element: "Errors: "+a.errors } );
		} else for (var i=ffrom; i<=fto && a.isCorrect; i+=fstep) {
			var args = new Array();
			args[0]=i;
// 			console.log("!!! " + i); 
			var expr = a.executeFunc(tmp, args);
			resultsModel.append( { element: i +" = "+ expr.expression } );
		}
		
		a.removeVariable(tmp);
	}
	
	Column {
		id: inputcol
		
		anchors.top: parent.top
		width: parent.width
		
		Grid {
			id: ins
			columns: 2
			
			Label {text: "Input: " } ExpressionInput { id: input; text: "sin x" }
			Label {text: "From:" }   RealInput { id: from; value: 0 }
			Label {text: "To:" }     RealInput { id: to; value: 10 }
			Label {text: "Step:" }   RealInput { id: step; value: 1 }
		}
		
		Button {
			text: "Go!"
			
			onClicked: calculateTable()
		}
		
		Label { text: "Results:"; id: res; }
	}
	
	SimpleListView {
		width: parent.width
		height: 200
		anchors.top: inputcol.bottom
		anchors.bottom: parent.bottom
		
		model: resultsModel
		role: "element"
	}
}
