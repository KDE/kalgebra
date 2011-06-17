import QtQuick 1.0


TextInput
{
	id: input
	color: "black"
	
	Keys.onPressed: {
		console.log("bum :: "+text);
// 		hints.text=text;
	}
	
// 	ToolTip {
// 		id: hints
// 	}
	height: input.font.pixelSize+5
}

// Rectangle
// {
// 	color: "green"
// }