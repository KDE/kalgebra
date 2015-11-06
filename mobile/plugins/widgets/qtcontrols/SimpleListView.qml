import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.1

ColumnLayout
{
    id: viewItem
    property string role: ""
    property string title: ""
    property alias currentIndex: table.currentRow
    property alias model: table.model
    property Component delegate
    property Component header
    property Component footer

    Loader {
        Layout.fillWidth: true
        sourceComponent: parent.header
    }

    TableView
    {
        id: table
        Layout.fillWidth: true
        Layout.fillHeight: true

        TableViewColumn {
            role: viewItem.role
            title: viewItem.title
        }

        itemDelegate: Item {
            Label {
                anchors.verticalCenter: parent.verticalCenter
                text: styleData.value
            }
        }
    }
    Loader {
        Layout.fillWidth: true
        sourceComponent: parent.footer
    }
}
