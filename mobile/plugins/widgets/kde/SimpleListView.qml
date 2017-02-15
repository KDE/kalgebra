import org.kde.kirigami 2.0
import QtQuick.Controls 1.1
import QtQuick 2.0

ListView
{
    id: scrollList
    property string role: ""
    property string title: ""
    delegate: BasicListItem { label: model[role] }
    header: Label { visible: ListView.view.title!=""; text: ListView.view.title }
    clip: true
}

