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
import QtQuick.Layouts 1.0
import org.kde.analitza 1.0
import org.kde.kirigami 2.5 as Kirigami
import org.kde.kalgebra.mobile 1.0
import QtQuick.Controls 2.5

KAlgebraPage {
    id: page

    function updateGraph() {
        view.model.clear();
        view.resetViewport();
        view.addFunction(operators.data(operators.index(chosebox.currentIndex,3)), App.variables);
    }

    ColumnLayout {
        id: descriptioncol

        anchors.fill: parent
        spacing: 10

        ComboBox {
            id: chosebox
            Layout.fillWidth: true
            textRole: "display"

            model: OperatorsModel {
                id: operators
            }

            onCurrentIndexChanged: {
                page.updateGraph();
            }
        }

        GridLayout {
            id: descgrid
            columns: 2
            Layout.fillWidth: true

            Label { text: operators.headerData(0,Qt.Horizontal) }
            Label { text: operators.data(operators.index(chosebox.currentIndex,0)) }
            Label { text: operators.headerData(1,Qt.Horizontal) }
            Label { text: operators.data(operators.index(chosebox.currentIndex,1)) }
            Label { text: operators.headerData(2,Qt.Horizontal) }
            Label { text: operators.data(operators.index(chosebox.currentIndex,2)) }
            Label { text: operators.headerData(3,Qt.Horizontal) }
            Label { text: operators.data(operators.index(chosebox.currentIndex,3)) }
        }

        Rectangle {
            color: 'white'
            Layout.fillWidth: true
            Layout.fillHeight: true

            Graph2D {
                id: view
                anchors {
                    fill: parent
                }
                model: PlotsModel {
                    id: plotsModel
                }
                Component.onCompleted: {
                    page.updateGraph();
                }
            }
        }
    }
}
