import QtQuick 2.0
import QtQuick.Controls 1.0

TableView
{
	id: viewItem
	property string role: ""
	property string title: ""
	property alias currentIndex: viewItem.currentRow
	property Component delegate
	
	TableViewColumn{
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
