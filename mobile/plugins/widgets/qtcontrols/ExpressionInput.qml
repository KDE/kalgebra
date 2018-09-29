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
import QtQuick.Window 2.0
import QtQuick.Controls 2.0
import org.kde.analitza 1.0
import org.kde.kalgebra.mobile 1.0

TextField
{
    id: field

    readonly property string currentWord: field.selectionStart === field.selectionEnd ? operators.lastWord(field.cursorPosition, field.text) : ""
    onCurrentWordChanged: view.currentIndex = -1

    Window {
        height: Math.min(100, view.contentHeight)
        width: field.width
        flags: Qt.ToolTip
        ListView {
            id: view
            anchors {
                fill: parent
                margins: 1
            }
            ScrollBar.vertical: ScrollBar {}
            currentIndex: -1
            model: QSortFilterProxyModel {
                sourceModel: OperatorsModel { id: operators }
                filterRegExp: new RegExp("^" + field.currentWord)
            }

            delegate: ItemDelegate {
                text: model.display + " - " + description
                highlighted: view.currentIndex === index
                width: ListView.view.width

                function complete() {
                    var toInsert = model.display.substr(field.currentWord.length);
                    if(isVariable)
                        toInsert += '(';
                    field.insert(field.cursorPosition, toInsert)
                }

                onClicked: complete()
                Keys.onReturnPressed: complete()
            }
        }

        visible: view.count >= 0 && field.activeFocus && field.currentWord.length > 0
        readonly property point globalPos: visible ? parent.mapToGlobal(field.x, field.y) : Qt.point(0,0)

        x: globalPos.x
        y: globalPos.y + field.height
    }

    placeholderText: i18n("Expression to calculate...")
    inputMethodHints: /*Qt.ImhPreferNumbers |*/ Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase

    Keys.forwardTo: view.visible && view.currentItem ? [ view.currentItem ] : null
    Keys.onUpPressed: view.decrementCurrentIndex()
    Keys.onDownPressed: view.incrementCurrentIndex()
    Keys.onReturnPressed: view.currentIndex = -1
}
