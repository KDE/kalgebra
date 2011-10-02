import com.nokia.meego 1.0

TextField
{
	id: input
	
	placeholderText: "Expression to calculate..."
	inputMethodHints: Qt.ImhPreferNumbers | Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
	
// 	Keys.onPressed: {
// 		console.log("bum :: "+text);
// 		hints.text=text;
// 	}
// 	
// 	ToolTip {
// 		id: hints
// 	}
// 	height: input.font.pixelSize+5
}

// Rectangle
// {
// 	color: "green"
// }
