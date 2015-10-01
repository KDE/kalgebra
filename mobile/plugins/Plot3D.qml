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
    ColumnLayout
    {
        anchors.fill: parent

        RowLayout {
            Layout.fillWidth: true

            ExpressionInput {
                id: input
                Layout.fillWidth: true

                text: "sin x*sin y"
                focus: true
                Component.onCompleted: selectAll()

                Keys.onReturnPressed: {
                    var err = view.addFunction(input.text, app.variables)
                    if (err.length>0)
                        console.warn("errors:", err)
                }
            }

            Button {
                id: exec
                text: "Clear"
                width: 100

                onClicked: {
                    view.model.clear()
                    view.resetView()
                    input.focus = true
                }
            }
        }
        Graph3D
        {
            id: view
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
