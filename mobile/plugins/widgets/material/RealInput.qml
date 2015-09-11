import Material 0.1
import QtQuick 2.0

TextField
{
	id: input
	property alias minimumValue: realvalidator.bottom
	property alias maximumValue: realvalidator.top
	
	property double value: parseFloat(text)
	inputMethodHints: Qt.ImhDigitsOnly | Qt.ImhNoPredictiveText
	
	validator: DoubleValidator { id: realvalidator }
	
	onTextChanged: value=parseFloat(text)
	
	Component.onCompleted: text=value
	
	Row {
		anchors {
			right: parent.right
			rightMargin: 2
		}
		spacing: 2
		height: parent.height-4
		y:2
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
