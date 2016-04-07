import org.kde.kirigami 1.0
import QtQuick 2.1

Page
{
    readonly property real dp: Units.devicePixelRatio
    title: "KAlgebra"
    default property alias contents: item.data

    Item {
        id: item
        anchors.fill: parent
    }
}
