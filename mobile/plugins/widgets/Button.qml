import QtQuick 1.0

Item {
	id: button
	signal clicked
	property bool toggled: false
	property alias text: text.text
	
	width: 100
	height: 60
	
	Rectangle
	{
		anchors.fill: button
		anchors.margins: mouseArea.pressed ? 3 : 2
		radius: mouseArea.pressed ? 8 : 6
		smooth: true
		gradient: Gradient {
			GradientStop { position: 0.0; color: "#003" }
			GradientStop { position: 1.0; color: "#004" }
		}
		
		Text
		{
			id: text
			anchors.centerIn: parent
// 			font.pixelSize: mouseArea.pressed ? 12 : 14
			color: "white"
			horizontalAlignment: Text.AlignHCenter
			verticalAlignment: Text.AlignVCenter
			
			style: Text.Sunken;
			styleColor: "black";
			smooth: true
		}
		
		MouseArea
		{
			id: mouseArea
			anchors.fill: parent
			hoverEnabled: true
			
			onClicked: {
				button.clicked()
			}
		}
	}
		
	Rectangle {
		id: shade
		anchors.fill: button; radius: 10; color: "black"; opacity: 0
		
		Behavior on opacity {
			NumberAnimation {
// 				properties: "opacity";
				duration: 100
			}
		}
	}
	
	states: [
		State {
			name: "pressed"; when: mouseArea.pressed == true
			PropertyChanges { target: shade; opacity: .4 }
		},
		
		State {
			name: "hovered"
			when: mouseArea.containsMouse == true
			PropertyChanges { target: shade; opacity: .2 }
		}
	]
}
