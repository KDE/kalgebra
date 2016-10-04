import QtQuick.Controls 2.1
import QtQuick 2.0

ListView
{
    id: scrollList
    property string role: ""
    property string title: ""
    delegate: ItemDelegate {
        Label {
            text: model[role]
        }
    }
    header: Label { visible: ListView.view.title!=""; text: ListView.view.title }
    clip: true
}

