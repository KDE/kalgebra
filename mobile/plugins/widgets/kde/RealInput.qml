import QtQuick 2.0
import org.kde.plasma.components 2.0

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