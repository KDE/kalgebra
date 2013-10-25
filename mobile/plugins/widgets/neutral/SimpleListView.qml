import QtQuick 2.0

ListView
{
	id: viewItem
	property string role: ""
	property string title
	
	SystemPalette { id: palette }
	clip: true
	header: Label {
		height: ListView.view.title=="" ? 0 : 1.5*implicitHeight
		text: ListView.view.title
		font.weight: Font.Bold
		verticalAlignment: Text.AlignVCenter
	}
	delegate: Label { text: model[role] }
	highlight: Rectangle { color: palette.midlight }
	
	ScrollDecorator {
		flickableItem: viewItem
	}
}
