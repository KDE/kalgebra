import org.kde.plasma.components 0.1
import QtQuick 1.1

Label {
	onLinkActivated: Qt.openUrlExternally(link)
}
