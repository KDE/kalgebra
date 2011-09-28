import QtQuick 1.1
import com.nokia.meego 1.0

TextField
{
	property double value: parseFloat(text)
	inputMethodHints: Qt.ImhDigitsOnly | Qt.ImhNoPredictiveText
	
	text: value.toString()
	
	validator: DoubleValidator {}
	
	onTextChanged: value=parseFloat(text)
}