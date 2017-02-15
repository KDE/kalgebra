import org.kde.kirigami 2.0
import QtQuick.Controls 1.1
import QtQuick 2.0

ListView
{
    id: scrollList
    property string role: ""
    property string title: ""
    delegate: BasicListItem { label: model[role] }
    header: scrollList.title ? titleComponent : null

    Component {
        id: titleComponent
        Label { text: ListView.view.title }
    }

    clip: true
}

