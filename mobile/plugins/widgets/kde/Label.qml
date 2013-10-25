import org.kde.plasma.components 2.0
import QtQuick 2.0

Label {
	onLinkActivated: Qt.openUrlExternally(link)
}
