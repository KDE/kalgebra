// SPDX-FileCopyrightText: 2015 by Aleix Pol <aleixpol@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import org.kde.analitza
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kalgebra.mobile

Kirigami.ScrollablePage {
    title: i18n("Value tables")

    property ListModel resultsModel: ListModel {}

    function showResults() {
        applicationWindow().pageStack.push("qrc:/TableResultPage.qml", {
            'results': resultsModel
        });
    }

    function calculateTable() {
        resultsModel.clear();

        var tmp = a.unusedVariableName()
        const lambda = a.dependenciesToLambda(input.text);
        if (lambda === "") {
            resultsModel.append( { element: i18n("Errors: Invalid expression %1", input.text) } );
            showResults();
            return;
        }
        var ret = a.insertVariable(tmp, lambda)
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
                const expr = a.executeFunc(tmp, [ i ]);
                if (typeof(expr) === "string") {
                    resultsModel.append( { element: i18n("error: %1", expr) } );
                } else {
                    resultsModel.append( { element: i +" = "+ expr.expression } );
                }
            }
        }

        a.removeVariable(tmp);
        showResults();
    }

    actions: QQC2.Action {
        icon.name: 'dialog-ok'
        text: i18nc("@action:button Run table", "Run")
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
            onAddOperation: calculateTable();
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
        QQC2.Button {
            text: i18n("Run")
            onClicked: calculateTable()
            visible: !Kirigami.Settings.isMobile
        }
    }
}
