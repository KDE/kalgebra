import QtQuick 2.0

TextInput
{
    SystemPalette { id: palette }
    
    height: input.font.pixelSize+5
    Rectangle {
        color: palette.base
        anchors.fill: parent
        z: parent.z-1
    }

//     placeholderText: "Expression to calculate..."
    inputMethodHints: Qt.ImhPreferNumbers | Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
}
