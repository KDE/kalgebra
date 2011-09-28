import com.nokia.meego 1.0
import org.kde.analitza 1.0
import QtQuick 1.1

PageStackWindow
{
	height: 400
	width: 300
	
	function goToPage(idx) {
		var toOpen = plugins.pluginPath(idx)
		
		try {
			var component = Qt.createComponent(toOpen)
			if (component.status == Component.Ready)
				pageStack.push(component);
			else
				console.log("Error loading component:", component.errorString());
		} catch (e) {
			console.log("error: "+e)
		}
	}
	
	Component {
		id: del
		
		Label {
			signal clicked
			text: display
			
			MouseArea {
				id: mouseArea;
				anchors.fill: parent
				onClicked: {
					goToPage(0)
				}
			}
		}
	}
	
	initialPage: ListView {
		id: pluginsView
		
		delegate: del
		
		model: PluginsModel { id: plugins }
	}
}
