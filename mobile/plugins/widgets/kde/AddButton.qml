import org.kde.kirigami 2.0 as Kirigami
import QtQuick 2.1

Item
{
    id: root
    signal clicked

    readonly property var act: Kirigami.Action {
        id: action
        iconName: "list-add"
        onTriggered: root.clicked()
    }

    Component.onCompleted: {
        var page = parent
        for (; page && page.title === undefined; page = page.parent) {}

        page.mainAction = action
    }
}
