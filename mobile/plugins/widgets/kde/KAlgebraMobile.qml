import QtQuick 2.0
import QtQuick.Window 2.1
import org.kde.plasma.components 2.0
import org.kde.kquickcontrolsaddons 2.0
import org.kde.analitza 1.0
import org.kde.kalgebra.mobile 1.0
import org.kde.plasma.core 2.0

Window {
	height: 400
	width: 300
	visible: true
	
	Rectangle {
		anchors.fill: parent
		color: "black"
		opacity: .2
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
	}
	
	Page {
		id: init
		anchors.margins: 10
		
		ListView {
			anchors.fill: parent
			spacing: 5
			
			move: Transition {
				NumberAnimation {
					properties: "x,y"
					easing.type: Easing.InOutQuad
				}
			}
			
			delegate: ListItem {
					height: 100
					enabled: true
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
							text: i18n("%1<br/>\n<em>%2</em>", title, subtitle)
						}
					}

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
		anchors.top: parent.top
		
		
		Image {
			anchors {
				left: parent.left
				verticalCenter: parent.verticalCenter
				leftMargin: 5
			}
			
			source: "qrc:/kde-edu-logo.png"
			height: parent.height
			fillMode: Image.PreserveAspectFit
			smooth: true
			
			MouseArea {
				anchors.fill: parent
				onClicked: Qt.openUrlExternally("http://edu.kde.org")
			}
		}
	}
	
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
