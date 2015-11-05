import Material 0.1
import QtQuick 2.1

Loader
{
    id: root
    property string iconName: ""
    property string text: ""

    signal clicked()

    Component {
        id: textComponent
        Button {
            text: root.text
            elevation: 1
        }
    }

    Component {
        id: iconComponent
        IconButton {
            iconName: root.iconName
        }
    }

    sourceComponent: root.text === "" ? iconComponent : textComponent

    Connections {
        target: root.item
        onClicked: root.clicked()
    }
}
