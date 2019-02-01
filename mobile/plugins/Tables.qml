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
import widgets 1.0
import QtQuick.Controls 2.5
import org.kde.kirigami 2.5 as Kirigami

KAlgebraPage
{
    Analitza {
        id: a
        variables: app.variables
    }
    
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
        
        if(fstep==0) {
            resultsModel.append( { element: i18n("Errors: The step cannot be 0") } );
        } else if(ffrom == fto) {
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
    }
    
    ColumnLayout {
        id: inputcol
        
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        spacing: 10
        
        GridLayout {
            id: ins
            columns: 2
            Layout.fillWidth: true
            
            Label { text: i18n("Input:") }
            ExpressionInput {
                id: input
                text: "sin x";
                Layout.fillWidth: true
                onAccepted: calculateTable();
            }
            Label { text: i18n("From:") }   RealInput { id: from; text: "0"; Layout.fillWidth: true; onAccepted: calculateTable() }
            Label { text: i18n("To:") }     RealInput { id: to; text: "10"; Layout.fillWidth: true; onAccepted: calculateTable() }
            Label { text: i18n("Step:") }   RealInput { id: step; text: "1"; Layout.fillWidth: true; onAccepted: calculateTable() }
        }
        
        Button {
            Layout.alignment: Qt.AlignRight
            text: i18n("Run")
            
            onClicked: calculateTable()
        }
    }
    
    Kirigami.CardsListView {
        width: parent.width
        anchors {
            top: inputcol.bottom
            bottom: parent.bottom
            topMargin: 5
        }
        currentIndex: -1
        
        model: ListModel { id: resultsModel }
        delegate: Kirigami.Card { contentItem: Label { text: model.element} }
        header: Label { text: i18n("Results:") }
    }
}
