import org.kde.kirigami 2.0
import QtQuick 2.1

Page
{
    readonly property real dp: Units.devicePixelRatio
    title: "KAlgebra"
    default property alias contents: item.data

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    Item {
        id: item
        anchors.fill: parent
    }
}
