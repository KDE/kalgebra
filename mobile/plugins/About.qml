import QtQuick 1.0
import org.kde.analitza 1.0
import "widgets"

KAlgebraPage
{
	Column {
		anchors.margins: 20
		anchors.fill: parent
		spacing: 20
		
		Image {
			source: "qrc:/kalgebra.svgz"
			anchors.horizontalCenter: parent.horizontalCenter
		}
		
		Label {
			anchors.left: parent.left
			anchors.right: parent.right
			
			wrapMode: Text.WordWrap
			horizontalAlignment: Text.AlignHCenter
			text: ("KAlgebra is brought to you by the lovely community of KDE and KDE Edu from a Free Software envionment.<br/>"+
					"If you have any problem with your software, please report it to <a href='http://bugs.kde.org'>our bug tracker</a>.<br /><br />"+
					"Aleix Pol Gonzalez &lt;<a href='mailto:aleixpol@kde.org'>aleixpol@kde.org</a>&gt;")
		}
		
		Image {
			source: "qrc:/kde-edu-logo.png"
			anchors.horizontalCenter: parent.horizontalCenter
		}
	}
}