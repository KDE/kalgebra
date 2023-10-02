// SPDX-FileCopyrightText: 2015 by Aleix Pol <aleixpol@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.analitza
import org.kde.kalgebra.mobile

Kirigami.ApplicationWindow
{
    id: rootItem
    height: 800
    width: Kirigami.Units.gridUnit * 70
    visible: true

    readonly property int columnWidth: Kirigami.Units.gridUnit * 13
    wideScreen: width > columnWidth * 5

    Kirigami.PagePool {
        id: mainPagePool
    }

    globalDrawer: Kirigami.OverlayDrawer {
        id: drawer

        edge: Qt.application.layoutDirection === Qt.RightToLeft ? Qt.RightEdge : Qt.LeftEdge

        modal: !rootItem.wideScreen
        onModalChanged: drawerOpen = !modal
        handleVisible: modal
        width: contentItem ? columnWidth : 0

        leftPadding: 0
        rightPadding: 0
        topPadding: 0
        bottomPadding: 0

        Kirigami.Theme.colorSet: Kirigami.Theme.View
        Kirigami.Theme.inherit: false

        drawerOpen: true

        readonly property list<T.Action> actions: [
            Kirigami.PagePoolAction {
                icon.name: "utilities-terminal"
                text: i18n("Calculator")
                page: "qrc:/Console.qml"
                pagePool: mainPagePool
            },
            Kirigami.PagePoolAction {
                id: show2dPlotAction
                icon.name: "draw-bezier-curves"
                text: i18n("Graph 2D")
                page: "qrc:/Plot2D.qml"
                pagePool: mainPagePool
            },
            Kirigami.PagePoolAction {
                id: show3dPlotAction
                icon.name: "adjustrgb"
                text: i18n("Graph 3D")
                page: "qrc:/Plot3D.qml"
                pagePool: mainPagePool
            },
            Kirigami.PagePoolAction {
                icon.name: "kspread"
                text: i18n("Value Tables")
                page: "qrc:/Tables.qml"
                pagePool: mainPagePool
            },
            Kirigami.PagePoolAction {
                icon.name: "document-properties"
                text: i18n("Variables")
                page: "qrc:/VariablesView.qml"
                pagePool: mainPagePool
            },
            Kirigami.PagePoolAction {
                icon.name: "documentation"
                text: i18n("Dictionary")
                page: "qrc:/Dictionary.qml"
                pagePool: mainPagePool
            },
            Kirigami.PagePoolAction {
                icon.name: "help-about"
                text: i18n("About KAlgebra")
                page: "qrc:/About.qml"
                pagePool: mainPagePool
            }
        ]

        contentItem: ColumnLayout {
            spacing: 0

            QQC2.ToolBar {
                Layout.fillWidth: true
                Layout.preferredHeight: rootItem.pageStack.globalToolBar.preferredHeight
                Layout.bottomMargin: Math.round(Kirigami.Units.smallSpacing / 2)

                leftPadding: Kirigami.Units.smallSpacing
                rightPadding: Kirigami.Units.smallSpacing
                topPadding: Kirigami.Units.smallSpacing
                bottomPadding: Kirigami.Units.smallSpacing

                contentItem: Kirigami.Heading {
                    text: i18n("KAlgebra")
                }
            }

            Repeater {
                delegate: Delegates.RoundedItemDelegate {
                    required property var modelData
                    action: modelData
                    Layout.fillWidth: true
                }
                model: drawer.actions
            }

            Item {
                Layout.fillHeight: true
            }
        }
    }

    readonly property Component customDrawer: Kirigami.OverlayDrawer {
        leftPadding: 0
        rightPadding: 0
        topPadding: 0
        bottomPadding: 0

        edge: Qt.application.layoutDirection == Qt.RightToLeft ? Qt.LeftEdge : Qt.RightEdge
        modal: !rootItem.wideScreen
        onModalChanged: drawerOpen = !modal
        drawerOpen: true
        onContentItemChanged: if (contentItem) {
            contentItem.visible = (mainPagePool.lastLoadedItem.drawerContent !== undefined);
        }
        width: mainPagePool.lastLoadedItem.drawerContent ? columnWidth : 0

        contentItem: mainPagePool.lastLoadedItem.drawerContent ?? null
        handleVisible: mainPagePool.lastLoadedItem.drawerContent !== undefined

        Kirigami.Theme.colorSet: Kirigami.Theme.View
        Kirigami.Theme.inherit: false
    }

    readonly property Component normalDrawer: Kirigami.ContextDrawer {}

    Component.onCompleted: if (Kirigami.Settings.isMobile) {
        contextDrawer = normalDrawer.createObject(rootItem);
    } else {
        contextDrawer = customDrawer.createObject(rootItem);
    }

    pageStack.initialPage: mainPagePool.loadPage("qrc:/Console.qml")
}
