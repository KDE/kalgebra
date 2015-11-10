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

    readonly property var iconConvert: {
        "utilities-terminal": "communication/dialpad",
        "draw-bezier-curves": "action/trending_up",
        "adjustrgb": "action/view_carousel",
        "kspread": "editor/format_list_numbered",
        "document-properties": "content/content_copy",
        "help-about": "action/help"
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

            width: Math.min(parent.width - Units.gu(1), Units.gu(7))
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
                delegate: ListItem.Subtitled {
                    text: title
                    subText: subtitle
                    selected: mainUi.source == model.path
                    iconName: rootItem.iconConvert[decoration]
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
