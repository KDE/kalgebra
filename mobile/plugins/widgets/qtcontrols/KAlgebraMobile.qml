import QtQuick 2.0
import QtQuick.Controls 1.0
import org.kde.analitza 1.0
import org.kde.kalgebra.mobile 1.0

ApplicationWindow
{
	id: rootItem
	height: 600
	width: 600
	
	function goToPage(path, deco) {
		try {
			var component = Qt.createComponent(path)
			if (component.status == Component.Ready)
				pageStack.push(component);
			else
				console.log("Error loading component:", component.errorString());
		} catch (e) {
			console.log("error: "+e)
		}
	}
	
	Component {
		id: pluginsView
		ListView {
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

	StackView {
		id: pageStack
		initialItem: pluginsView
	}
}
