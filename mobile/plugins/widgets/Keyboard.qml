import QtQuick 1.0

Row {
	Grid {
		id:nums
		columns: 3
		rows: 4
		height: 300
		
		signal newInput(string text)
		signal deleteChar
		signal clear
		
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
		
		Button { text: "AC"; onClicked: doSelectAll();}
		
		anchors.bottom: nums.bottom
		anchors.top: nums.top
	}
}