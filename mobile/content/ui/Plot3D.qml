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

import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.0
import org.kde.analitza 1.1
import QtQuick.Controls 2.5
import org.kde.kalgebra.mobile 1.0
import org.kde.kirigami 2.5 as Kirigami

KAlgebraPage {
    id: page

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    FileDialog {
        id: fileDialog
        folder: shortcuts.home
        onAccepted: proceed()

        property var proceed
    }

    contextualActions: [
        Action {
            text: i18n("Save")
            onTriggered: {
                fileDialog.title = text
                fileDialog.proceed = function() {
                    var ret = view.save(fileDialog.fileUrl)
                    console.log("saved 3D", fileDialog.fileUrl, ret)
                }
                fileDialog.nameFilters = view.filters
                fileDialog.selectExisting = false
                fileDialog.open()
            }
        },
        Action {
            text: i18n("Reset Viewport")
            onTriggered: view.resetViewport()
        }
    ]

    Graph3D
    {
        id: view
        anchors.fill: parent
        model: App.functionsModel()

        Kirigami.OverlaySheet {
            id: dialog

            header: RowLayout {
                width: parent.width
                ExpressionInput {
                    id: input
                    Layout.fillWidth: true
                    text: "sin x*sin y"
                    focus: true
                    Component.onCompleted: selectAll()
                    onAccepted: {
                        input.selectAll()
                        var err = App.functionsModel().addFunction(input.text, 4, App.variables)
                        if (err.length>0)
                            console.warn("errors:", err)
                    }
                }
                Button {
                    icon.name: "list-add"
                    onClicked: {
                        input.selectAll()
                        var err = view.addFunction(input.text, App.variables)
                        if (err.length>0)
                            console.warn("errors:", err)
                    }
                }
            }

            contentItem: Kirigami.CardsListView {
                id: list
                delegate: Kirigami.Card {
                    contentItem: Label { text: model.description }
                }
                model: view.model

                footer: Button {
                    text: i18n("Clear All")
                    onClicked: {
                        view.model.clear()
                        view.resetView()
                    }
                }
            }
        }

        AddButton {
            onClicked: {
                dialog.open();
            }
        }
    }
}
