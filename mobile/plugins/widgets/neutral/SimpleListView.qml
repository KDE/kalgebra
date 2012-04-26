import QtQuick 1.1

ListView
{
	id: viewItem
	property string role: ""
	property string title
	
	clip: true
	header: Label {
		height: ListView.view.title=="" ? 0 : 1.5*implicitHeight
		text: ListView.view.title
		font.weight: Font.Bold
		verticalAlignment: Text.AlignVCenter
	}
	delegate: Label { text: model[role] }
	
	ScrollDecorator {
		flickableItem: viewItem
	}
}
