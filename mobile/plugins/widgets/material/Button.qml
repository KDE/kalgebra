import Material 0.1
import QtQuick 2.1
import QtQuick.Layouts 1.1

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
            Layout.minimumWidth: implicitWidth*2
            Layout.minimumHeight: implicitHeight*2
            iconName: root.iconName
        }
    }

    Layout.minimumWidth: item.Layout.minimumWidth
    Layout.minimumHeight: item.Layout.minimumHeight

    sourceComponent: root.text === "" ? iconComponent : textComponent

    Connections {
        target: root.item
        onClicked: root.clicked()
    }
}
