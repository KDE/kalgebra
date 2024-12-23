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

import QtQuick
import QtQuick.Window
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.analitza
import QtQuick.Layouts
import org.kde.kalgebra.mobile

QQC2.TextArea
{
    id: field
    signal addOperation(operation: string)

    readonly property string currentWord: field.cursorPosition <= field.text.length ? operators.lastWord(field.cursorPosition, field.text) : ""
    property var history: []
    property int historyPos: 0

    Layout.preferredHeight: Kirigami.Units.gridUnit * 2

    placeholderText: i18n("Expression to calculate...")
    inputMethodHints: /*Qt.ImhPreferNumbers |*/ Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase

    Keys.forwardTo: view.visible && view.currentIndex >= 0 ? [ view.currentItem ] : null
    Keys.onTabPressed: view.incrementCurrentIndex()
    Keys.onUpPressed: {
        if (view.count > 0) {
            view.decrementCurrentIndex()
        } else {
            if (historyPos < (history.length - 1))
                historyPos++
            text = history[history.length - 1 - historyPos]
        }
    }
    Keys.onDownPressed: {
        if (view.count > 0) {
            view.incrementCurrentIndex()
        } else {
            if (historyPos > 0)
                historyPos--
            text = history[history.length - 1 - historyPos]
        }
    }
    Keys.onReturnPressed: {
        view.currentIndex = -1;
        historyPos = -1
        history.push(text)
        addOperation(text);
        field.text = '';
    }

    QQC2.Popup {
        id: popup
        visible: view.count > 0
        parent: QQC2.Overlay.overlay

        readonly property point fieldPos: field.parent.mapToGlobal(field.x, field.y)
        x: fieldPos.x
        y: fieldPos.y - popup.height
        width: field.width
        height: Math.min(Kirigami.Units.gridUnit * 13, view.count * Kirigami.Units.gridUnit * 2 + popup.topPadding + popup.bottomPadding)

        ListView {
            id: view
            currentIndex: -1
            anchors.fill: parent
            keyNavigationWraps: true
            clip: true
            model: field.currentWord.length > 0 ? filterModel : []

            QSortFilterProxyModel {
                id: filterModel
                sourceModel: OperatorsModel { id: operators }
                filterRegularExpression: new RegExp("^" + field.currentWord)
                filterCaseSensitivity: Qt.CaseInsensitive
            }

            delegate: Delegates.RoundedItemDelegate {
                required property int index
                required property string description
                required property string name
                required property bool isVariable

                text: name + " - " + description
                highlighted: view.currentIndex === index
                width: ListView.view.width

                function complete() {
                    let toInsert = name.substr(field.currentWord.length);
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

