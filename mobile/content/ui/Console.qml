// SPDX-FileCopyrightText: 2015 by Aleix Pol <aleixpol@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQml.Models
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.analitza
import org.kde.kalgebra.mobile

Kirigami.ScrollablePage {
    id: page

    title: i18n("Calculator")
    ListModel { id: itemModel }

    // This content is only available in the desktop version of Kalgebra.
    // Don't put any information here that can't be accessed by another
    // part of Kalgebra.
    readonly property Item drawerContent : ColumnLayout {
        visible: true
        width: 300
        spacing: 0

        QQC2.ToolBar {
            Layout.fillWidth: true
            Layout.preferredHeight: applicationWindow().pageStack.globalToolBar.preferredHeight

            leftPadding: Kirigami.Units.smallSpacing
            rightPadding: Kirigami.Units.smallSpacing
            topPadding: Kirigami.Units.smallSpacing
            bottomPadding: Kirigami.Units.smallSpacing

            contentItem: Kirigami.Heading {
                text: i18n("KAlgebra")
            }
        }

        QQC2.ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ListView {
                model: VariablesModel { variables: App.variables }
                delegate: Delegates.RoundedItemDelegate {
                    required property string whatsThis
                    required property int index

                    text: whatsThis
                }
            }
        }
    }

    actions: [
        Kirigami.Action {
            text: i18nc("@action:button", "Load Script")
            onTriggered: {
                fileDialog.title = text
                fileDialog.proceed = function() { consoleModel.loadScript(fileDialog.fileUrl) }
                fileDialog.nameFilters = [ i18n("Script (*.kal)") ]
		fileDialog.fileMode = FileDialog.OpenFile
                fileDialog.open()
            }
        },
        Kirigami.Action {
            text: i18nc("@action:button", "Save Script")
            onTriggered: {
                fileDialog.title = text
                fileDialog.proceed = function() { consoleModel.saveScript(fileDialog.fileUrl) }
                fileDialog.nameFilters = [ i18n("Script (*.kal)") ]
		fileDialog.fileMode = FileDialog.SaveFile
                fileDialog.open()
            }
        },
        //TODO: Recent scripts
        Kirigami.Action {
            text: i18nc("@action:button", "Export Log")
            onTriggered: {
                fileDialog.title = text
                fileDialog.proceed = function() { consoleModel.saveLog(fileDialog.fileUrl) }
		fileDialog.nameFilters = [ i18n("HTML (*.html)") ]
		fileDialog.fileMode = FileDialog.SaveFile
                fileDialog.open()
            }
        },
        // --
        Kirigami.Action {
            text: consoleModel.mode === 1 ? i18nc("@action:button", "Evaluate") : i18nc("@action:button", "Calculate")
            onTriggered: consoleModel.mode = consoleModel.mode === 1 ? 0 : 1
        },
        // --
        Kirigami.Action {
            icon.name: "edit-clear-history"
            text: i18nc("@action:button", "Clear Log")
            onTriggered: itemModel.clear()
            enabled: itemModel.count !== 0
        }
    ]

    ListView {
        id: view
        model: itemModel

        bottomMargin: Kirigami.Units.largeSpacing
        topMargin: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.largeSpacing

        delegate: FormCard.FormCard {
            width: ListView.view.width
            FormCard.AbstractFormDelegate {
                background: null
                contentItem: RowLayout {
                    QQC2.Label {
                        text: model.result
                        onLinkActivated: {
                            input.remove(input.selectionStart, input.selectionEnd)
                            input.insert(input.cursorPosition, consoleModel.readContent(link))
                        }

                        Layout.fillWidth: true
                    }

                    QQC2.ToolButton {
                        visible: App.functionsModel().canAddFunction(expression, 2, App.variables)
                        text: i18n("2D Plot")
                        onClicked: {
                            App.functionsModel().addFunction(expression, 2, App.variables)
                            show2dPlotAction.trigger();
                        }
                    }

                    QQC2.ToolButton {
                        visible: App.functionsModel().canAddFunction(expression, 4, App.variables)
                        text: i18n("3D Plot")
                        onClicked: {
                            App.functionsModel().addFunction(expression, 4, App.variables)
                            show3dPlotAction.trigger();
                        }
                    }

                    QQC2.ToolButton {
                        readonly property string value: result.replace(/<[^>]*>/g, '');
                        text: i18n("Copy \"%1\"", value)
                        icon.name: "edit-copy"
                        display: QQC2.ToolButton.IconOnly
                        onClicked: {
                            clipboard.content = value
                        }

                        QQC2.ToolTip.text: text
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                    }
                }
            }
        }
        Clipboard {
            id: clipboard
        }

        ConsoleModel {
            id: consoleModel
            variables: App.variables
            onMessage: (msg, operation, result) => {
                itemModel.append({ result: msg, expression: result.toString()  })
                input.selectAll()
                view.currentIndex = view.count-1
                view.positionViewAtIndex(view.currentIndex, ListView.Contain)
            }
        }

        FileDialog {
            id: fileDialog
            currentFolder: StandardPaths.standardLocations(StandardPaths.HomeLocation)[0]
            onAccepted: proceed()

            property var proceed
        }

        Kirigami.PlaceholderMessage {
            text: i18nc("@info", "Empty console history")
            visible: view.count === 0
            anchors.centerIn: parent
            width: parent.width - Kirigami.Units.gridUnit * 4
        }
    }

    footer: ExpressionInput {
        id: input
        focus: true

        width: page.width

        onAddOperation: (content) => {
            consoleModel.addOperation(content)
        }
    }
}
