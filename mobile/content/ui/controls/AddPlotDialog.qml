// SPDX-FileCopyrightText: 2021 Swapnil Tripathi <swapnil06.st@gmail.com>
// SPDX-FileCopyrightText: 2023 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.kirigamiaddons.components as Components

import org.kde.analitza
import org.kde.kalgebra.mobile

QQC2.Dialog {
    id: root

    property alias text: input.text
    required property int dimension

    width: Math.min(parent.width - Kirigami.Units.gridUnit * 2, Kirigami.Units.gridUnit * 30)
    height: Math.min(parent.height - Kirigami.Units.gridUnit * 2, Kirigami.Units.gridUnit * 30)

    x: Math.round((parent.width - width) / 2)
    y: Math.round((parent.height - height) / 2)
    z: Kirigami.OverlayZStacking.z

    parent: applicationWindow().QQC2.Overlay.overlay

    background: Components.DialogRoundedBackground {}

    padding: 0
    modal: true

    contentItem: ColumnLayout {
        spacing: 0
        RowLayout {
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.smallSpacing
            spacing: Kirigami.Units.smallSpacing

            QQC2.TextField {
                id: input
                Layout.fillWidth: true
                focus: true
                Component.onCompleted: selectAll()
                Keys.onReturnPressed: {
                    input.selectAll()
                    App.functionsModel().addFunction(input.text, root.dimension, App.variables)
                }
            }

            QQC2.Button {
                icon.name: "list-add"
                text: i18nc("@action:button", "Add")
                display: QQC2.Button.IconOnly
                onClicked: {
                    input.selectAll()
                    App.functionsModel().addFunction(input.text, root.dimension, App.variables)
                }

                QQC2.ToolTip.text: text
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            }

            QQC2.Button {
                text: i18nc("@action:button", "Clear All")
                onClicked: {
                    App.functionsModel().clear();
                    view.resetViewport();
                }

                QQC2.ToolTip.text: text
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            }

            QQC2.ToolButton {
                icon.name: "dialog-close"
                text: i18nc("@action:button", "Close dialog")
                display: QQC2.Button.IconOnly
                onClicked: {
                    root.close()
                }

                QQC2.ToolTip.text: text
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            }
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        QQC2.ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ListView {
                id: plotView

                model: App.functionsModel()
                delegate: Delegates.RoundedItemDelegate {
                    id: plotDelegate

                    required property int index
                    required property string description

                    text: description

                    contentItem: RowLayout {
                        Delegates.DefaultContentItem {
                            itemDelegate: plotDelegate
                            Layout.fillWidth: true
                        }

                        QQC2.ToolButton {
                            icon.name: "list-remove"
                            text: i18nc("@action:button", "Remove plot")
                            display: QQC2.Button.IconOnly

                            onClicked: App.functionsModel().removeRows(plotDelegate.index, 1);

                            QQC2.ToolTip.text: text
                            QQC2.ToolTip.visible: hovered
                            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                        }
                    }
                }

                Kirigami.PlaceholderMessage {
                    width: parent.width - Kirigami.Units.gridUnit * 4
                    anchors.centerIn: parent
                    text: i18nc("@info", "No plot available")
                    visible: plotView.count === 0
                }
            }
        }
    }
}
