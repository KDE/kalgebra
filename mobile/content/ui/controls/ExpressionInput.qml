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

ColumnLayout {
    spacing: 0

    property alias text: field.text

    signal addOperation(operation: string)

    function selectAll(): void {
        field.selectAll();
    }

    function remove(start: int, end: int): void {
        field.remove(start, end);
    }

    function insert(start: int, text: string): void {
        field.insert(start, text);
    }

    Kirigami.Separator {
        Layout.fillWidth: true
    }

    ListView {
        id: view
        currentIndex: -1
        keyNavigationWraps: true
        Layout.fillWidth: true
        Layout.preferredHeight: Math.min(contentHeight, Kirigami.Units.gridUnit * 10)
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

    Kirigami.Separator {
        Layout.fillWidth: true
        visible: view.count > 0
    }

    QQC2.TextField {
        id: field

        focus: true

        readonly property string currentWord: operators.lastWord(field.cursorPosition, field.text)

        Layout.minimumWidth: parent.width
        Layout.preferredHeight: Kirigami.Units.gridUnit * 2

        background: null

        placeholderText: i18n("Expression to calculate...")
        inputMethodHints: /*Qt.ImhPreferNumbers |*/ Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase

        Keys.forwardTo: view.visible && view.currentIndex >= 0 ? [ view.currentItem ] : null
        Keys.onTabPressed: view.incrementCurrentIndex()
        Keys.onUpPressed: view.decrementCurrentIndex()
        Keys.onDownPressed: view.incrementCurrentIndex()
        Keys.onReturnPressed: {
            view.currentIndex = -1;
            addOperation(text);
            field.text = '';
        }
    }
}

