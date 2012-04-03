import QtQuick 1.1

TextInput
{
    Rectangle { color: "green"; anchors.fill: parent; z: parent.z-1}
    
	property double value: parseFloat(text)
	inputMethodHints: Qt.ImhDigitsOnly | Qt.ImhNoPredictiveText
	
	text: value.toString()
	
	validator: DoubleValidator {}
	
	onTextChanged: value=parseFloat(text)
}