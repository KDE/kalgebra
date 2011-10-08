import QtQuick 1.1
import org.kde.analitza 1.0

Rectangle {
	width: parent.width
	height: 200
	color: 'white'
	
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
// 			onPinchUpdated: {
// 				var doScale = scale/previousScale;
// 				view.scale(doScale)
// 			}
// 		}
		
		MouseArea {
			anchors.fill: parent
			property int lastX: 0
			property int lastY: 0
			acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.MiddleButton
			
			onPressed: { lastX=mouse.x; lastY=mouse.y }
			
			onPositionChanged: {
				view.translate(mouse.x-lastX, mouse.y-lastY)
				
				lastX=mouse.x
				lastY=mouse.y
			}
		}
	}
}