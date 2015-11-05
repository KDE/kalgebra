import Material 0.1

ActionButton
{
    id: root
    signal triggered()

    anchors {
        right: parent.right
        bottom: parent.bottom
        margins: Units.dp(16)
    }

    action: Action {
        onTriggered: root.triggered()
    }
    iconName: "content/add"
}
