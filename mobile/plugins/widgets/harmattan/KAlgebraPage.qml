import com.nokia.meego 1.0

Page
{
	anchors.margins: rootWindow.pageMargin
	
	tools: ToolBarLayout {
		id: pageStackWindowTools
		visible: true
		ToolIcon { iconId: "toolbar-back"; onClicked: pageStack.pop(); }
	}
}
