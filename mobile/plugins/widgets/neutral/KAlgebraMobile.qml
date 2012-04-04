import org.kde.analitza 1.0
import QtQuick 1.1

Rectangle
{
	id: rootItem
	color: "white"
	height: 600
	width: 600
	
	function goToPage(path) {
		try {
			var component = Qt.createComponent(path)
			if (component.status == Component.Ready) {
				var obj = component.createObject(rootItem)
// 				obj.z = rootItem+1
				obj.anchors.fill = rootItem
				
			} else {
				console.log("Error loading component:", component.errorString());
			}
		} catch (e) {
			console.log("error: "+e)
		}
	}
	
	ListView {
		id: pluginsView
		anchors.fill: parent
		spacing: 10
		header: Image {
			source: "qrc:/kalgebra.svgz"
			anchors.horizontalCenter: parent.horizontalCenter
		}
		footer: Image {
			source: "qrc:/kde-edu-logo.png"
			anchors.horizontalCenter: parent.horizontalCenter
		}
		delegate:
			Button {
				text: title
				onClicked: goToPage(model.path)
				anchors.horizontalCenter: parent.horizontalCenter
			}

		
		model: PluginsModel { id: plugins }
	}
}
