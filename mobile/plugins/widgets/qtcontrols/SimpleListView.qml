import QtQuick.Controls 2.1
import QtQuick 2.0

ListView
{
    id: scrollList
    property string role: ""
    property string title: ""
    ScrollBar.vertical: ScrollBar {}
    delegate: ItemDelegate {
        Label {
            text: model[role]
        }
    }
    header: Label { visible: ListView.view.title!=""; text: ListView.view.title }
    clip: true
}

