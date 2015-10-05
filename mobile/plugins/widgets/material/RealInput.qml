import Material 0.1
import QtQuick 2.0
import QtQuick.Layouts 1.0

RowLayout
{
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

    Button {
        text: "+"
        elevation: 1
        onClicked: {
            if(input.value+1<maximumValue) {
                input.text=(input.value+1)
            }
        }
    }

    Button {
        text: "-"
        elevation: 1
        onClicked: {
            if(input.value-1>minimumValue) {
                input.text=(input.value-1)
            }
        }
    }
}
