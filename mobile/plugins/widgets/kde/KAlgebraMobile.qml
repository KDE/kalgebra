import QtQuick 1.0
import org.kde.plasma.components 0.1
import org.kde.plasma.extras 0.1
import org.kde.qtextracomponents 0.1
import org.kde.analitza 1.0
import org.kde.plasma.core 0.1

Item {
	height: 400
	width: 300
	
	Image {
		anchors.fill: parent
		source: "image://appbackgrounds/standard"
		fillMode: Image.Tile
		asynchronous: true
	}

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
		
		GridView {
			id: pluginsView
			anchors.fill: parent
			cellHeight: 100
			cellWidth: 200
			
			delegate:
				ListItem {
					width: pluginsView.cellWidth
					height: pluginsView.cellHeight
					Column {
						anchors.fill: parent
						spacing: 10
						
						QIconItem {
							id: icon
							width: 50
							height: 50
							anchors.horizontalCenter: parent.horizontalCenter
							icon: decoration
						}
						
						Text {
							id: description
							anchors.left: parent.left
							anchors.right: parent.right
							horizontalAlignment: Text.AlignHCenter
							text: i18n("%1<br/>\n%2", title, subtitle)
						}
					}
					
					MouseArea {
						anchors.fill: parent
						onClicked: goToPage(model.path, decoration)
					}
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
			bottom: parent.bottom
		}
		
		initialPage: init
			
		toolBar: toolBar
	}
}
