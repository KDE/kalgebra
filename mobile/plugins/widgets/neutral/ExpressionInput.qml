import QtQuick 1.1

TextInput
{
	height: input.font.pixelSize+5
    Rectangle {
        color: "blue"
        anchors.fill: parent
        z: parent.z-1
        opacity: 0.1
    }
    
// 	placeholderText: "Expression to calculate..."
	inputMethodHints: Qt.ImhPreferNumbers | Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
}
