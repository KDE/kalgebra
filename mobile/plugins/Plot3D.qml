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
import org.kde.analitza 1.1
import widgets 1.0

KAlgebraPage
{
    id: page
    Graph3D
    {
        id: view
        anchors.fill: parent

        Dialog {
            id: dialog
            height: Math.min((4*view.height)/5, list.contentHeight)+page.dp*10

            SimpleListView {
                id: list
                anchors {
                    fill: parent
                    margins: page.dp*4
                }
                role: "description"
                model: view.model

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
                            var err = view.addFunction(input.text, app.variables)
                            if (err.length>0)
                                console.warn("errors:", err)
                        }
                    }
                    Button {
                        iconName: "content/add"
                        onClicked: {
                            input.selectAll()
                            var err = view.addFunction(input.text, app.variables)
                            if (err.length>0)
                                console.warn("errors:", err)
                        }
                    }
                }

                footer: Button {
                    text: "Clear All"
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
