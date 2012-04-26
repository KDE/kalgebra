import com.nokia.meego 1.0
import QtQuick 1.1

Label {
	onLinkActivated: Qt.openUrlExternally(link)
}
