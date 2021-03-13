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
import QtQuick.Controls 2.14 as QQC2
import org.kde.kirigami 2.14 as Kirigami
import org.kde.analitza 1.0
import QtQuick.Layouts 1.2
import org.kde.kalgebra.mobile 1.0

QQC2.TextField {
    id: field
    readonly property string currentWord: operators.lastWord(field.cursorPosition, field.text)

    QQC2.Popup {
        id: popupCompletion
        x: 0
        y: -height - Kirigami.Units.smallSpacing
        width: parent.width
        visible: view.count > 0 && field.activeFocus && field.currentWord.length > 0
        topPadding: 0
        leftPadding: 0
        bottomPadding: 0
        rightPadding: 0
        height: Math.min(Kirigami.Units.gridUnit * 10, view.contentHeight)
        contentItem: QQC2.ScrollView {
            ListView {
                id: view
                currentIndex: 0
                keyNavigationWraps: true
                model: QSortFilterProxyModel {
                    sourceModel: OperatorsModel { id: operators }
                    filterRegularExpression: new RegExp("^" + field.currentWord)
                    filterCaseSensitivity: Qt.CaseInsensitive
                }

                delegate: Kirigami.BasicListItem {
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
        }
    }

    placeholderText: i18n("Expression to calculate...")
    inputMethodHints: /*Qt.ImhPreferNumbers |*/ Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase

    Keys.forwardTo: view.visible && view.currentItem ? [ view.currentItem ] : null
    Keys.onTabPressed: view.incrementCurrentIndex()
    Keys.onUpPressed: view.decrementCurrentIndex()
    Keys.onDownPressed: view.incrementCurrentIndex()
    Keys.onReturnPressed: view.currentIndex = -1
}

