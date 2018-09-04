import org.kde.kirigami 2.5
import QtQuick.Controls 2.1
import QtQuick 2.0

CardsListView
{
    id: scrollList

    ScrollBar.vertical: ScrollBar {}

    property string role: ""
    property string title: ""
    delegate: Card {
        contentItem: Label { text: model[scrollList.role] }
    }
    header: scrollList.title ? titleComponent : null

    Component {
        id: titleComponent
        Label { text: scrollList.title }
    }

    clip: true
}

