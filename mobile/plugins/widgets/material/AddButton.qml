import Material 0.1

ActionButton
{
    id: root
    signal clicked()

    anchors {
        right: parent.right
        bottom: parent.bottom
        margins: Units.dp(16)
    }

    action: Action {
        onTriggered: root.clicked()
    }
    iconName: "content/add"
}
