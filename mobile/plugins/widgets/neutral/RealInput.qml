import QtQuick 1.1

TextInput
{
	Rectangle { color: "green"; anchors.fill: parent; z: parent.z-1; opacity: 0.3 }
	
	property double value: parseFloat(text)
	inputMethodHints: Qt.ImhDigitsOnly | Qt.ImhNoPredictiveText
	
	validator: DoubleValidator {}
	
	onTextChanged: value=parseFloat(text)
	
	Component.onCompleted: text=value
}