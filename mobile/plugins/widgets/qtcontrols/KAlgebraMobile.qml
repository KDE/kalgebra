import QtQuick 2.2
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.1
import org.kde.analitza 1.0
import org.kde.kalgebra.mobile 1.0

ApplicationWindow
{
    id: rootItem
    height: 600
    width: 600
    visible: true

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            ToolButton {
                text: "\u2630"
                onClicked: drawer.open()
            }
            Label {
                text: "KAlgebra"
            }
            Item {
                Layout.fillWidth: true
            }
        }
    }

    property string currentPath: pluginsModel.pluginPath(0)
    onCurrentPathChanged: {
        var component = Qt.createComponent(currentPath);
        if (component.status == Component.Error) {
            console.log("error", component.errorString());
            return;
        }

        try {
            stack.replace(component)
        } catch(e) {
            console.log("error", e)
        }
        drawer.position = 0
    }

    Drawer {
        id: drawer
        edge: Qt.LeftEdge
        width: parent.width - 56 * 1
        height: parent.height

        ColumnLayout {
            anchors.fill: parent
            Repeater {
                delegate: ItemDelegate {
                    Layout.fillWidth: true
                    text: title
                    highlighted: model.path == rootItem.currentPath
                    onClicked: {
                        rootItem.currentPath = model.path
                    }
                }
                model: PluginsModel {
                    id: pluginsModel
                }
            }

            Item {
                width: 5
                Layout.fillHeight: true
            }
        }
    }

    StackView {
        id: stack
        anchors.fill: parent
    }
}
