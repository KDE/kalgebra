import QtQuick 2.0
import org.kde.analitza 1.0
import widgets 1.0

KAlgebraPage
{
	ListModel { id: resultsModel }
	Analitza {
		id: a
		variables: app.variables
	}
	
	function calculateTable() {
		resultsModel.clear();
		
		var tmp = a.unusedVariableName()
		var ret = a.insertVariable(tmp, a.dependenciesToLambda(input.text))
		var ffrom=from.value, fto=to.value, fstep=step.value;
// 		console.log("chancho (" + ffrom + ", " + fto + " : " + fstep + ") " + ret);
		if((fto-ffrom>0)!=(fstep>0)) {
			fstep *= -1;
			step.value = fstep
		}
// 		console.log("chancho2 (" + ffrom + ", " + fto + " : " + fstep + ") " + ret);
		
		if(fstep==0) {
			resultsModel.append( { element: "Errors: The step can't be 0" } );
		} else if(!a.isCorrect) {
			resultsModel.append( { element: "Errors: "+ (ret ? ret : a.errors) } );
		} else {
			for (var i=ffrom; i<=fto && a.isCorrect; i+=fstep) {
				var args = new Array();
				args[0]=i;
	// 			console.log("!!! " + i); 
				var expr = a.executeFunc(tmp, args);
				resultsModel.append( { element: i +" = "+ expr.expression } );
			}
		}
		
		a.removeVariable(tmp);
	}
	
	Column {
		id: inputcol
		
		anchors {
			top: parent.top
			left: parent.left
			right: parent.right
		}
		spacing: 10
		
		Grid {
			id: ins
			columns: 2
			spacing: 5
			width: parent.width
			
			Label { text: "Input:" }  ExpressionInput { id: input; text: "sin x"; width: ins.width-input.x-5; onAccepted: calculateTable() }
			Label { text: "From:" }   RealInput { id: from; value: 0; width: ins.width-from.x-5; onAccepted: calculateTable() }
			Label { text: "To:" }     RealInput { id: to; value: 10; width: ins.width-to.x-5; onAccepted: calculateTable() }
			Label { text: "Step:" }   RealInput { id: step; value: 1; width: ins.width-step.x-5; onAccepted: calculateTable() }
		}
		
		Button {
			text: "Go!"
			
			onClicked: calculateTable()
		}
	}
	
	SimpleListView {
		width: parent.width
		anchors {
			top: inputcol.bottom
			bottom: parent.bottom
			topMargin: 5
		}
		currentIndex: -1
		
		model: resultsModel
		role: "element"
		title: "Results:"
	}
}
