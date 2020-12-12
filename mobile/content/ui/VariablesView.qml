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
import org.kde.analitza 1.0
import org.kde.kirigami 2.5 as Kirigami
import org.kde.kalgebra.mobile 1.0
import QtQuick.Controls 2.5 as QQC2


Kirigami.ScrollablePage {
    title: i18n("Variables")
    Kirigami.CardsListView {
        anchors.fill: parent
        model: VariablesModel { variables: App.variables }
        currentIndex: -1
        delegate: Kirigami.Card {
            contentItem: QQC2.Label { text: model.whatsThis }
        }
    }
}
