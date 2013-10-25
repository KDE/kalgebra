import QtQuick 2.0

Text {
	SystemPalette { id: palette }
	color: palette.text
	onLinkActivated: Qt.openUrlExternally(link)
}
