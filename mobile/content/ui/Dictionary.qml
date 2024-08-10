// SPDX-FileCopyrightText: 2015 by Aleix Pol <aleixpol@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import org.kde.analitza
import org.kde.kirigami as Kirigami
import org.kde.kalgebra.mobile
import QtQuick.Controls

Kirigami.Page {
    id: page

    title: i18nc("@title:window", "Dictionary")

    function updateGraph() {
        view.model.clear();
        view.resetViewport();
        view.addFunction(operators.data(operators.index(chosebox.currentIndex,3)), App.variables);
    }
    ColumnLayout {
        anchors.fill: parent
        spacing: 15

        Kirigami.FormLayout {
            id: layout

            ComboBox {
                id: chosebox
                Kirigami.FormData.label: i18n("Name:")
                textRole: "display"

                model: OperatorsModel {
                    id: operators
                }

                onCurrentIndexChanged: {
                    page.updateGraph();
                }
            }
            Label {
                text: operators.data(operators.index(chosebox.currentIndex,0))
                Kirigami.FormData.label: i18n("%1:", operators.headerData(0,Qt.Horizontal))
            }
            Label {
                text: operators.data(operators.index(chosebox.currentIndex,1))
                Kirigami.FormData.label: i18n("%1:", operators.headerData(1,Qt.Horizontal))
            }
            Label {
                text: operators.data(operators.index(chosebox.currentIndex,2))
                Kirigami.FormData.label: i18n("%1:", operators.headerData(2,Qt.Horizontal))
            }
            Label {
                text: operators.data(operators.index(chosebox.currentIndex,3))
                Kirigami.FormData.label: i18n("%1:", operators.headerData(3,Qt.Horizontal))
            }
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
