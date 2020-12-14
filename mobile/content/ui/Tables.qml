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
import org.kde.analitza 1.0
import QtQuick.Controls 2.5
import org.kde.kirigami 2.14 as Kirigami
import org.kde.kalgebra.mobile 1.0

Kirigami.ScrollablePage {
    title: i18n("Value tables")

    property ListModel resultsModel: ListModel {}

    function calculateTable() {
        resultsModel.clear();

        var tmp = a.unusedVariableName()
        var ret = a.insertVariable(tmp, a.dependenciesToLambda(input.text))
        var ffrom = from.realValue, fto=to.realValue, fstep=step.realValue;
//         console.log("chancho (" + ffrom + ", " + fto + " : " + fstep + ") " + ret);
        if((fto-ffrom>0)!=(fstep>0)) {
            fstep *= -1;
            step.value = fstep
        }
//         console.log("chancho2 (" + ffrom + ", " + fto + " : " + fstep + ") " + ret);

        if(fstep===0) {
            resultsModel.append( { element: i18n("Errors: The step cannot be 0") } );
        } else if(ffrom === fto) {
            resultsModel.append( { element: i18n("Errors: The start and end are the same") } );
        } else if(!a.isCorrect) {
            resultsModel.append( { element: i18n("Errors: %1", ret ? ret : a.errors) } );
        } else {
            for (var i=ffrom; i<=fto &&  i>=ffrom && a.isCorrect; i+=fstep) {
                var args = new Array();
                args[0]=i;
                var expr = a.executeFunc(tmp, args);
                resultsModel.append( { element: i +" = "+ expr.expression } );
            }
        }

        a.removeVariable(tmp);
        if (applicationWindow().pageStack.depth > 1) {
            applicationWindow().pageStack.replace("qrc:/TableResultPage.qml", {
                'results': resultsModel
            });
        } else {
            applicationWindow().pageStack.push("qrc:/TableResultPage.qml", {
                'results': resultsModel
            });
        }
    }

    actions.main: Kirigami.Action {
        icon.name: 'dialog-ok'
        text: i18n('Run')
        onTriggered: calculateTable()
    }

    Analitza {
        id: a
        variables: App.variables
    }

    Kirigami.FormLayout {
        ExpressionInput {
            Kirigami.FormData.label: i18n("Input")
            id: input
            text: "sin x";
            Layout.fillWidth: true
            onAccepted: calculateTable();
        }
        RealInput {
            id: from;
            text: "0";
            Kirigami.FormData.label: i18n("From:")
            Layout.fillWidth: true;
            onAccepted: calculateTable()
        }
        RealInput {
            id: to;
            text: "10";
            Kirigami.FormData.label: i18n("To:")
            Layout.fillWidth: true;
            onAccepted: calculateTable()
        }

        RealInput {
            id: step;
            Kirigami.FormData.label: i18n("Step")
            text: "1";
            Layout.fillWidth: true;
            onAccepted: calculateTable()
        }
        Button {
            text: i18n("Run")
            onClicked: calculateTable()
            visible: !Kirigami.Settings.isMobile
        }
    }
}
