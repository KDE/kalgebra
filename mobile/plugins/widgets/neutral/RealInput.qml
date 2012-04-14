import QtQuick 1.1

TextInput
{
	id: input
	SystemPalette { id: palette }
	
	Rectangle { color: palette.base; anchors.fill: parent; z: parent.z-1; opacity: 1 }
	
	property double value: parseFloat(text)
	inputMethodHints: Qt.ImhDigitsOnly | Qt.ImhNoPredictiveText
	
	validator: DoubleValidator {}
	
	onTextChanged: value=parseFloat(text)
	
	Component.onCompleted: text=value
	
	Row {
		anchors.right: parent.right
		height: parent.height
		Button { text: "+"; onClicked: input.text=(input.value+1); height: parent.height }
		Button { text: "-"; onClicked: input.text=(input.value-1); height: parent.height }
	}
}