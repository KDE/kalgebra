import QtQuick 1.1
import com.nokia.meego 1.0

ListView
{
    id: view
    property string role: ""
    property string title: ""
    header: Label { visible: ListView.view.title!=""; text: ListView.view.title }
    delegate: Label { text: model[role] }
    ScrollDecorator {
        flickableItem: view
    }
    clip: true
}
