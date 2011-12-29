import com.nokia.meego 1.0
import com.nokia.extras 1.0
import org.kde.analitza 1.0
import QtQuick 1.1

PageStackWindow
{
	height: 400
	width: 300
	
	function goToPage(path) {
// 		var toOpen = plugins.pluginPath(idx)
		
		try {
			var component = Qt.createComponent(path)
			if (component.status == Component.Ready)
				pageStack.push(component);
			else {
				console.log("Error loading component:", component.errorString());
				errorDialog.message = component.errorString()
				errorDialog.open()
			}
		} catch (e) {
			console.log("error: "+e)
			errorDialog.message = e
			errorDialog.open()
		}
	}
	
	Dialog {
		id: errorDialog
		property alias message: text.text
		
		content:Item {
			id: name
			height: 50
			width: parent.width
			Label {
				id: text
				font.pixelSize: 22
				anchors.fill: parent
				wrapMode: Text.WordWrap
				color: "white"
			}
		}
   }
	
	initialPage: 
		Page {
			anchors.margins: UiConstants.DefaultMargin
			
			ListView {
				id: pluginsView
				anchors.fill: parent
				
				delegate:
					ListDelegate {
						Image {
							source: "image://theme/icon-m-common-drilldown-arrow" + (theme.inverted ? "-inverse" : "")
							anchors.right: parent.right;
							anchors.verticalCenter: parent.verticalCenter
						}
						
						onClicked: goToPage(model.path)
					}

				
				model: PluginsModel { id: plugins }
			}
		}
}
