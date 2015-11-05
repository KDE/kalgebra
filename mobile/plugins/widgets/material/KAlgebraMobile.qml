import QtQuick 2.2
import QtQuick.Layouts 1.2
import Material 0.1
import Material.ListItems 0.1 as ListItem
import org.kde.analitza 1.0
import org.kde.kalgebra.mobile 1.0

ApplicationWindow
{
    id: rootItem
    height: 600
    width: 600
    visible: true

    theme {
        primaryColor: Palette.colors["blue"]["500"]
        primaryDarkColor: Palette.colors["blue"]["700"]
        accentColor: Palette.colors["blue"]["A200"]
        tabHighlightColor: "white"
    }
    
    initialPage: Page {
        title: "KAlgebra"

        tabs: navDrawer.enabled ? [] : plugins.titles
        onSelectedTabChanged: {
            mainUi.source = plugins.pluginPath(selectedTab);
        }

        backAction: navDrawer.action

        NavigationDrawer {
            id: navDrawer

            enabled: parent.width < Units.dp(700)

            ListView {
                spacing: 10
                anchors.fill: parent
                header: Rectangle {
                    width: parent.width
                    height: navDrawer.height/5
                    color: "#4A66A0"
                    Image {
                        anchors {
                            left: parent.left
                            right: parent.right

                            verticalCenter: parent.verticalCenter
                            leftMargin: Units.dp(16)
                            rightMargin: Units.dp(64)
                            verticalCenterOffset: -height/20
                        }
                        fillMode: Image.PreserveAspectFit

                        source: "qrc:/kde-edu-logo.png"
                    }
                }
                delegate: ListItem.Standard {
                    text: title
                    selected: mainUi.source == model.path
                    onClicked: {
                        mainUi.source = model.path
                        navDrawer.close()
                    }
                }
                model: PluginsModel { id: plugins }
            }
        }

        Loader {
            id: mainUi
            anchors.fill: parent
            source: plugins.pluginPath(0)
        }
    }
}
