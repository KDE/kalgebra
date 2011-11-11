import org.kde.plasma.components 0.1

Page {
	id: page
	height: 100
	width: 100
	
	anchors.margins: 10
	
	tools: ToolBarLayout {
		ToolButton {
			iconSource: "go-previous"
			text: i18n("Back")
			
			onClicked: page.pageStack.pop()
		}
	}
}
