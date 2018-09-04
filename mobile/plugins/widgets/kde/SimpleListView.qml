import org.kde.kirigami 2.5
import QtQuick.Controls 2.1
import QtQuick 2.0

ListView
{
    id: scrollList

    ScrollBar.vertical: ScrollBar {}

    property string role: ""
    property string title: ""

    spacing: Units.largeSpacing * 2
    topMargin: Units.largeSpacing
    bottomMargin: Units.largeSpacing

    delegate: Card {
        width: scrollList.width - Units.largeSpacing * 4
        x: Units.largeSpacing * 2

        contentItem: Label { text: model[scrollList.role] }
    }
    header: scrollList.title ? titleComponent : null

    Component {
        id: titleComponent
        Label { text: scrollList.title }
    }

    clip: true
}

