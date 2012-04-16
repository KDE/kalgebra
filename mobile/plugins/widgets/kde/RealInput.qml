import QtQuick 1.1
import org.kde.plasma.components 0.1

TextField
{
	property double value: parseFloat(text)
	property alias minimumValue: realvalidator.bottom
	property alias maximumValue: realvalidator.top
	inputMethodHints: Qt.ImhDigitsOnly | Qt.ImhNoPredictiveText
	
	text: value.toString()
	
	validator: DoubleValidator { id: realvalidator }
	
	onTextChanged: value=parseFloat(text)
	
	Component.onCompleted: text=value
}