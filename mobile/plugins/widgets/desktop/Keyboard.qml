import QtQuick 1.0

Row {
	signal clear
	
	Grid {
		id:nums
		columns: 3
		rows: 4
		height: 300
		anchors.top: parent.top
		anchors.bottom: parent.bottom
		
		signal newInput(string text)
		signal deleteChar
		
		CalcButton { text: "1" }
		CalcButton { text: "2" }
		CalcButton { text: "3" }
		CalcButton { text: "4" }
		CalcButton { text: "5" }
		CalcButton { text: "6" }
		CalcButton { text: "7" }
		CalcButton { text: "8" }
		CalcButton { text: "9" }
		CalcButton { text: "." }
		CalcButton { text: "0" }
	}
	
	Flow {
		flow: Flow.TopToBottom
		
		CalcButton { text: "-" }
		CalcButton { text: "+" }
		CalcButton { text: "*" }
		CalcButton { text: "/" }
		CalcButton { text: "(" }
		CalcButton { text: ")" }
		
		Button { text: "AC"; onClicked: clear();}
		
		anchors.bottom: nums.bottom
		anchors.top: nums.top
	}
}