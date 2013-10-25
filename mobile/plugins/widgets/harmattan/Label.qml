import com.nokia.meego 1.0
import QtQuick 2.0

Label {
	onLinkActivated: Qt.openUrlExternally(link)
}
