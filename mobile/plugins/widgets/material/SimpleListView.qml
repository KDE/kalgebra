import QtQuick 2.0
import Material 0.1
import Material.ListItems 0.1 as ListItem

ListView
{
    id: viewItem
    property string role: ""
    property string title
    clip: true

    header: ListItem.Subheader {
        text: ListView.view.title
    }
    delegate: ListItem.Standard {
        text: model[role]
        interactive: false
    }

    Scrollbar {
        flickableItem: viewItem
    }
}
