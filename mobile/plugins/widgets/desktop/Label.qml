import QtDesktop 0.1
import QtQuick 2.0

Label {
	onLinkActivated: Qt.openUrlExternally(link)
}
