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

import org.kde.kirigami 2.5
import QtQuick.Controls 2.1
import QtQuick 2.0

ListView
{
    id: scrollList

    ScrollBar.vertical: ScrollBar {}

    property string role: ""
    property string title: ""

    spacing: Units.largeSpacing * 2
    topMargin: Units.largeSpacing
    bottomMargin: Units.largeSpacing

    delegate: Card {
        width: scrollList.width - Units.largeSpacing * 4
        x: Units.largeSpacing * 2

        hiddenActions: model.hiddenActions || []
        contentItem: Label { text: model[scrollList.role] }
    }
    header: scrollList.title ? titleComponent : null

    Component {
        id: titleComponent
        Label { text: scrollList.title }
    }

    clip: true
}

