import QtQuick 1.1

ListView
{
	id: viewItem
	property string role: ""
	property string title
	
	header: Label { visible: ListView.view.title!=""; text: ListView.view.title }
	delegate: Label { text: model[role] }
}
