import QtQuick 2.0

Item
{
	Button {
		text: "<-"
		width: 50
		height: 50
		anchors {
			bottom: parent.bottom
			right: parent.right
		}
		onClicked: pageStack.pop()
		z: parent.z+1
	}
}
