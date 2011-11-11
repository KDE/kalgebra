import org.kde.plasma.components 0.1
import QtQuick 1.0

ListView
{
	id: scrollList
	property string role: ""
	delegate: Label { text: model[role] }
	clip: true
	
	ScrollBar {
		id: scrollBar
		orientation: Qt.Vertical
		flickableItem: scrollList
		stepSize: 40
		scrollButtonInterval: 50
		anchors {
			top: scrollList.top
			right: scrollList.right
			bottom: scrollList.bottom
		}
	}
}

