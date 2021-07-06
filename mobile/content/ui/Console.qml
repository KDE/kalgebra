/*************************************************************************************
 *  Copyright (C) 2015 by Aleix Pol <aleixpol@kde.org>                               *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

import org.kde.kirigami 2.14 as Kirigami
import QtQuick 2.2
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.5 as QQC2
import QtQml.Models 2.10
import QtQuick.Dialogs 1.0
import org.kde.analitza 1.0
import org.kde.kalgebra.mobile 1.0

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
        Kirigami.AbstractApplicationHeader {
            Layout.fillWidth: true
            topPadding: Kirigami.Units.smallSpacing
            bottomPadding: Kirigami.Units.largeSpacing
            leftPadding: Kirigami.Units.smallSpacing
            rightPadding: Kirigami.Units.smallSpacing
            Layout.preferredHeight: Kirigami.Units.gridUnit * 2
            Kirigami.Heading {
                level: 1
                text: i18n("Variables")
                Layout.fillWidth: true
            }
        }
        QQC2.ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            ListView {
                model: VariablesModel { variables: App.variables }
                delegate: Kirigami.BasicListItem {
                    label: model.whatsThis
                }
            }
        }
    }

    contextualActions: [
        Kirigami.Action {
            text: i18n("Load Script")
            onTriggered: {
                fileDialog.title = text
                fileDialog.proceed = function() { consoleModel.loadScript(fileDialog.fileUrl) }
                fileDialog.nameFilters = [ i18n("Script (*.kal)") ]
                fileDialog.selectExisting = true
                fileDialog.open()
            }
        },
        Kirigami.Action {
            text: i18n("Save Script")
            onTriggered: {
                fileDialog.title = text
                fileDialog.proceed = function() { consoleModel.saveScript(fileDialog.fileUrl) }
                fileDialog.nameFilters = [ i18n("Script (*.kal)") ]
                fileDialog.selectExisting = false
                fileDialog.open()
            }
        },
        //TODO: Recent scripts
        Kirigami.Action {
            text: i18n("Export Log")
            onTriggered: {
                fileDialog.title = text
                fileDialog.proceed = function() { consoleModel.saveLog(fileDialog.fileUrl) }
                fileDialog.nameFilters = [ i18n("HTML (*.html)") ]
                fileDialog.selectExisting = false
                fileDialog.open()
            }
        },
        // --
        Kirigami.Action {
            text: consoleModel.mode === ConsoleModel.Calculate ? i18n("Evaluate") : i18n("Calculate")
            onTriggered: consoleModel.mode = consoleModel.mode === ConsoleModel.Calculate ? ConsoleModel.Evaluate : ConsoleModel.Calculate
        },
        // --
        Kirigami.Action {
            iconName: "edit-clear-history"
            text: i18n("Clear Log")
            onTriggered: itemModel.clear()
            enabled: itemModel.count != 0
        }
    ]
    
    Kirigami.CardsListView {
        id: view
        model: itemModel

        delegate: Kirigami.Card {
            contentItem: QQC2.Label {
                text: model.result
                onLinkActivated: {
                    input.remove(input.selectionStart, input.selectionEnd)
                    input.insert(input.cursorPosition, consoleModel.readContent(link))
                }
            }

            actions: [
                Kirigami.Action {
                    visible: App.functionsModel().canAddFunction(expression, 2, App.variables)
                    text: i18n("2D Plot")
                    onTriggered: {
                        App.functionsModel().addFunction(expression, 2, App.variables)
                        show2dPlotAction.trigger();
                    }
                },
                Kirigami.Action {
                    visible: App.functionsModel().canAddFunction(expression, 4, App.variables)
                    text: i18n("3D Plot")
                    onTriggered: {
                        App.functionsModel().addFunction(expression, 4, App.variables)
                        show3dPlotAction.trigger();
                    }
                },
                Kirigami.Action {
                    readonly property string value: result.replace(/<[^>]*>/g, '');
                    text: i18n("Copy \"%1\"", value)
                    icon.name: "edit-copy"
                    displayHint: Kirigami.DisplayHint.AlwaysHide
                    onTriggered: {
                        clipboard.content = value
                    }
                }
            ]
        }
        Clipboard {
            id: clipboard
        }

        ConsoleModel {
            id: consoleModel
            variables: App.variables
            onMessage: {
                itemModel.append({ result: msg, expression: result.toString()  })
                input.selectAll()
                view.currentIndex = view.count-1
                view.positionViewAtIndex(view.currentIndex, ListView.Contain)
            }
        }

        FileDialog {
            id: fileDialog
            folder: shortcuts.home
            onAccepted: proceed()

            property var proceed
        }

    }

    footer: ExpressionInput {
        id: input
        focus: true

        Keys.onReturnPressed: {
            consoleModel.addOperation(text)
        }
    }
}
