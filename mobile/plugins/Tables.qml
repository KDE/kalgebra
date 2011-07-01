import QtQuick 1.0
import "widgets"

Rectangle
{
	id: bg
	
	ListModel
	{
		id: resultsModel
	}
	
	Column {
		anchors.fill: parent
		
		Row { Text {text: "Input:" } ExpressionInput {} }
		Row { Text {text: "From:" } RealInput { id: from; text: "0" } }
		Row { Text {text: "To:" } RealInput { id: to; text: "100" } }
		
		Button {
			text: "Go!" 
			onClicked: {
				resultsModel.clear();
		
// 				var tmp = a.unusedVariableName();
				var ffrom=parseFloat(from.text), fto=parseFloat(to.text);
				console.log("chancho " + ffrom + ", " + fto);
				
				for (var i=ffrom; i<=fto; i++) {
					var args = new Array();
					args[0]=i;
					console.log("!!! " + i); 
					/*+a.executeFunc(tmp, args))*/
					resultsModel.append( { value: ": "/*; text: ":D"*/ } );
				}
				
// 				a.removeVariable(tmp);
			}
		}
		
		Text { text: "Results:" }
		ListView {
			model: resultsModel
			height: 100
			width: bg.width
			delegate: Text { text: value/*+" :: "+text */}
		}
	}
}
