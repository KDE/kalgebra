// SPDX-FileCopyrightText: 2021 Swapnil Tripathi <swapnil06.st@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.0
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.8 as Kirigami

import org.kde.analitza 1.0
import org.kde.kalgebra.mobile 1.0

Kirigami.OverlayDrawer {
    id: drawer
    parent: page
    height: parent.height / 2
    edge: Qt.BottomEdge
    z: -1

    Behavior on height {
        NumberAnimation { duration: Kirigami.Units.shortDuration }
    }
    contentItem: ColumnLayout {
        anchors.fill: parent

        ToolBar {
            Layout.fillWidth: true

            RowLayout {
                anchors.fill: parent

                ExpressionInput {
                    id: input
                    Layout.fillWidth: true
                    text: "sin x"
                    focus: true
                    Component.onCompleted: selectAll()
                    Keys.onReturnPressed: {
                        input.selectAll()
                        view.addFunction(input.text, App.variables)
                    }
                }
                Button {
                    icon.name: "list-add"
                    onClicked: {
                        input.selectAll()
                        view.addFunction(input.text, App.variables)
                    }
                }
                Button {
                    text: i18n("Clear All")
                    onClicked: {
                        App.functionsModel().clear();
                        view.resetViewport();
                    }
                }
                Button {
                    icon.name: "collapse"
                    onClicked: {
                        drawer.close()
                    }
                }
            }
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            ListView {
                model: App.functionsModel()
                delegate: Kirigami.SwipeListItem {
                    contentItem: Label {
                        text: model.description
                    }
                }
            }
        }
    }
}
