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
		
		
		PinchArea {
			anchors.fill: parent
			property double thePreviousScale: 1
// 			dragAxis: YAxis
			
// 			onPinchFinished
// 			onPinchStarted: 
			
			function distance(p1, p2)
			{
				var w=p2.x-p1.x
				var h=p2.y-p1.y
				return Math.sqrt(w*w+h*h)
			}
			
			onPinchStarted: thePreviousScale=1
			
			onPinchUpdated: {
				var currentDistance = distance(pinch.point1, pinch.point2)
				if(currentDistance>0) {
					var startDistance = distance(pinch.startPoint1, pinch.startPoint2)
					
					var theCurrentScale = startDistance/currentDistance
					
					var doScale = theCurrentScale/thePreviousScale
					view.scale(doScale, view.width/2, view.height/2)
// 					console.log("scale...", startDistance, theCurrentScale, doScale)
					thePreviousScale = theCurrentScale
				}
			}
		}
		
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