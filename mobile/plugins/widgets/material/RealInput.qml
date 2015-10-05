import Material 0.1
import QtQuick 2.0
import QtQuick.Layouts 1.0

RowLayout
{
    id: root
    property alias text: input.text
    property alias minimumValue: realvalidator.bottom
    property alias maximumValue: realvalidator.top
    readonly property double value: parseFloat(input.text)

    signal accepted()

    TextField
    {
        id: input
        Layout.fillWidth: true

        inputMethodHints: Qt.ImhDigitsOnly | Qt.ImhNoPredictiveText

        validator: DoubleValidator { id: realvalidator }

        Component.onCompleted: text=value

        onAccepted: parent.accepted()
    }

    IconButton {
        iconName: "hardware/keyboard_arrow_up"
        onClicked: {
            if(root.value+1<realvalidator.top) {
                input.text=(root.value+1)
            }
        }
    }

    IconButton {
        iconName: "hardware/keyboard_arrow_down"
        onClicked: {
            if(root.value-1>realvalidator.bottom) {
                input.text=(root.value-1)
            }
        }
    }
}
