import QtQuick 1.1

TextInput
{
	id: input
	property alias minimumValue: realvalidator.bottom
	property alias maximumValue: realvalidator.top
	SystemPalette { id: palette }
	
	Rectangle { color: palette.base; anchors.fill: parent; z: parent.z-1; opacity: 1 }
	
	property double value: parseFloat(text)
	inputMethodHints: Qt.ImhDigitsOnly | Qt.ImhNoPredictiveText
	
	validator: DoubleValidator { id: realvalidator }
	
	onTextChanged: value=parseFloat(text)
	
	Component.onCompleted: text=value
	
	Row {
		anchors.right: parent.right
		height: parent.height
		Button {
			text: "+"
			height: parent.height
			onClicked: {
				if(input.value+1<maximumValue) {
					input.text=(input.value+1)
				}
			}
		}
		Button {
			text: "-"
			onClicked: {
				if(input.value-1>minimumValue) {
					input.text=(input.value-1)
				}
			}
			height: parent.height
		}
	}
}