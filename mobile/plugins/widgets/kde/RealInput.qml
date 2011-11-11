import QtQuick 1.1
import org.kde.plasma.components 0.1

TextField
{
	property double value: parseFloat(text)
	inputMethodHints: Qt.ImhDigitsOnly | Qt.ImhNoPredictiveText
	
	text: value.toString()
	
	validator: DoubleValidator {}
	
	onTextChanged: value=parseFloat(text)
}