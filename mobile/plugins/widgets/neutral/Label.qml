import QtQuick 1.1

Text {
	SystemPalette { id: palette }
	color: palette.text
	onLinkActivated: Qt.openUrlExternally(link)
}
