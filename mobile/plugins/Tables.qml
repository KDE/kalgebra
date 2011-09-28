import QtQuick 1.0
import org.kde.analitza 1.0
import "widgets"

Rectangle
{
	id: bg
	height: 200
	width: 200
	
	ListModel { id: resultsModel }
	Analitza { id: a }
	
	function calculateTable() {
		resultsModel.clear();
		
		var tmp = a.unusedVariableName();
		var ret = a.execute(tmp+":="+input.text, true)
		var ffrom=parseFloat(from.text), fto=parseFloat(to.text), fstep=parseFloat(step.text);
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
		Text {text: "From:" }   RealInput { id: from; text: "0" }
		Text {text: "To:" }     RealInput { id: to; text: "10" }
		Text {text: "Step:" }   RealInput { id: step; text: "1" }
		
		Button {
			text: "Go!"
			
			onClicked: calculateTable()
		}
	}
	
	Text { text: "Results:"; id: res; anchors.top: ins.bottom }
	
	ListView {
		id: view
		anchors.bottom: parent.bottom
		anchors.top: res.bottom
		
		model: resultsModel
		width: bg.width
		delegate: Text { text: value+" -> "+result}
	}
}
