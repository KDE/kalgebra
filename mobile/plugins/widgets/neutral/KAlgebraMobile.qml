import org.kde.analitza 1.0
import QtQuick 2.0
import QtQuick.Window 2.1
import org.kde.kalgebra.mobile 1.0

Window
{
	id: rootItem
	SystemPalette { id: palette }
	color: palette.window
	height: 600
	width: 600
	visible: true
	
	function goToPage(path) {
		try {
			var component = Qt.createComponent(path)
			if (component.status == Component.Ready) {
				var obj = component.createObject(rootItem)
// 				obj.z = rootItem+1
				obj.anchors.fill = pluginsView
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
			source: "qrc:/kalgebra.png"
			anchors.horizontalCenter: parent.horizontalCenter
			
			MouseArea {
				anchors.fill: parent
				onClicked: Qt.openUrlExternally("http://edu.kde.org/applications/mathematics/kalgebra/")
			}
		}
		footer: Image {
			source: "qrc:/kde-edu-logo.png"
			anchors.horizontalCenter: parent.horizontalCenter
			
			MouseArea {
				anchors.fill: parent
				onClicked: Qt.openUrlExternally("http://edu.kde.org")
			}
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
