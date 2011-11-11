import QtQuick 1.0
import org.kde.plasma.components 0.1
import org.kde.analitza 1.0

Rectangle {
	height: 400
	width: 300
	color: "lightgrey"
	
	function goToPage(path, deco) {
// 		var toOpen = plugins.pluginPath(idx)
		
		try {
			var component = Qt.createComponent(path)
			if (component.status == Component.Ready)
				pageStack.push(component);
			else
				console.log("Error loading component:", component.errorString());
		} catch (e) {
			console.log("error: "+e)
		}
// 		d.text=deco
	}
	
	Page {
		id: init
		anchors.margins: 10
		
		ListView {
			id: pluginsView
			anchors.fill: parent
			
			delegate:
				ToolButton {
					iconSource: decoration
					text: i18n("%1 - %2", title, subtitle)
					
					onClicked: goToPage(model.path, decoration)
				}

			
			model: PluginsModel { id: plugins }
		}
		
		tools: ToolBarLayout {}
	}
	
	ToolBar {
		id: toolBar
		z: 10
		width: parent.width
		height: 40
		anchors {
			top: parent.top
		}
	}
	
// 	IconWidget {
// 		id: d
// 		height: toolBar.height-5
// 		width: height
// 		anchors.top: parent.top
// 		anchors.left: parent.left
// 		anchors.margins: 5
// 		anchors.leftMargin: 20
// 		
// 		source: "kalgebra"
// 		
// 	}
	
	PageStack
	{
		id: pageStack
		width: parent.width
		anchors {
			top: toolBar.bottom
		}
		
		initialPage: init
			
		toolBar: toolBar
	}
}
