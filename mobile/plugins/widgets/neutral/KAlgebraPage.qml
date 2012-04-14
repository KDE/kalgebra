import QtQuick 1.1

Rectangle
{
	SystemPalette { id: palette }
	color: palette.window
	
	Button {
		text: "<-"
		anchors {
			bottom: parent.bottom
			right: parent.right
		}
		onClicked: parent.destroy()
		z: parent.z+1
	}
}
