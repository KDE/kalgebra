import org.kde.plasma.components 0.1

Rectangle {
	id: tooltip
	property string text
	Text {
		text: tooltip.text
		
		anchors.fill: parent
	}
	
	color: "blue"
}