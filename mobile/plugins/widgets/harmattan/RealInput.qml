import QtQuick 1.1
import com.nokia.meego 1.0

TextField
{
    property double value: parseFloat(text)
	text: value.toString()
	
	validator: DoubleValidator {}
	
	onTextChanged: value=parseFloat(text)
}