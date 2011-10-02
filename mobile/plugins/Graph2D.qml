import QtQuick 1.1
import org.kde.analitza 1.0

Rectangle {
	width: parent.width
	height: 200
	color: 'white'
	
	function getKeys (obj) {
		var keys = [];
		for(var key in obj)
			keys.push(key);
		
		return keys;
	}
	
	Graph2DView {
		id: view
		anchors.fill: parent
		
		squares: true
		model: app.functionsModel()
		
// 		PinchArea {
// 			anchors.fill: parent
// 			dragAxis: YAxis
// 			
// // 			onPinchFinished
// 			onPinchStarted: 
// 			
// 			onPinchUpdated:
// 		}
		
		MouseArea {
			anchors.fill: parent
			property int lastX: 0
			property int lastY: 0
			
			onPressed: { lastX=mouse.x; lastY=mouse.y }
			
			onPositionChanged: {
				view.translate(mouse.x-lastX, mouse.y-lastY)
				
				lastX=mouse.x
				lastY=mouse.y
			}
		}
	}
}