import QtQuick 2.2
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
        primaryColor: Palette.colors["red"]["500"]
        primaryDarkColor: Palette.colors["blue"]["700"]
        accentColor: Palette.colors["red"]["A200"]
        tabHighlightColor: "white"
    }
    
    initialPage: Page {
        title: "KAlgebra"

        tabs: navDrawer.enabled ? [] : plugins.titles
        onSelectedTabChanged: {
            var path = plugins.pluginPath(selectedTab);
            mainUi.source = path
        }

        backAction: navDrawer.action
        NavigationDrawer {
            id: navDrawer

            enabled: parent.width < Units.dp(700)

            ListView {
                spacing: 10
                anchors.fill: parent
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
