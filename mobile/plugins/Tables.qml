import QtQuick 1.0
import "widgets"

Rectangle
{
	ListModel
	{
		id: resultsModel
		ListElement { expression: "cooooh"; value: "loooo" }
	}
	
	Column {
		anchors.fill: parent
		
		Row { Text {text: "Input:" } ExpressionInput {} }
		Row { Text {text: "From:" } RealInput { id: from; text: "0" } }
		Row { Text {text: "To:" } RealInput { id: to; text: "100" } }
		
		Button { text: "Go!" 
			onClicked: {
				resultsModel.clear();
		
// 				var tmp = a.unusedVariableName();
				var from=Math.parseFloat(from.value), to=parseFloat(to.value);
				console.log("chancho " + from + ", " + to);
				
				for (var i=from; i<=to; i++) {
					var args = new Array();
					args[0]=i;
					console.log("!!! " + i); 
					resultsModel.addItem(i+": "/*+a.executeFunc(tmp, args)*/);
				}
				
// 				a.removeVariable(tmp);
			}
		}
		
		Text { text: "Results:" }
		ListView {
			model: resultsModel
		}
	}
}
