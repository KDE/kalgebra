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

import QtQuick 2.2
import QtQuick.Layouts 1.2
import org.kde.kirigami 2.14 as Kirigami
import org.kde.analitza 1.0
import org.kde.kalgebra.mobile 1.0

Kirigami.ApplicationWindow
{
    id: rootItem
    height: 800
    width: Kirigami.Units.gridUnit * 70
    visible: true

    readonly property int columnWidth: Kirigami.Units.gridUnit * 13
    wideScreen: width > columnWidth * 5

    Kirigami.PagePool {
        id: mainPagePool
    }

    globalDrawer: Kirigami.GlobalDrawer {
        id: drawer

        modal: !rootItem.wideScreen
        onModalChanged: drawerOpen = !modal
        handleVisible: modal

        title: "KAlgebra"
        titleIcon: "qrc:/kalgebra.svgz"
        drawerOpen: true

        actions: [
            Kirigami.PagePoolAction {
                icon.name: "utilities-terminal"
                text: i18n("Calculator")
                page: "qrc:/Console.qml"
                pagePool: mainPagePool
            },
            Kirigami.PagePoolAction {
                icon.name: "draw-bezier-curves"
                text: i18n("Graph 2D")
                page: "qrc:/Plot2D.qml"
                pagePool: mainPagePool
            },
            Kirigami.PagePoolAction {
                icon.name: "adjustrgb"
                text: i18n("Graph 3D")
                page: "qrc:/Plot3D.qml"
                pagePool: mainPagePool
            },
            Kirigami.PagePoolAction {
                icon.name: "kspread"
                text: i18n("Value Tables")
                page: "qrc:/Tables.qml"
                pagePool: mainPagePool
            },
            Kirigami.PagePoolAction {
                icon.name: "document-properties"
                text: i18n("Variables")
                page: "qrc:/VariablesView.qml"
                pagePool: mainPagePool
            },
            Kirigami.PagePoolAction {
                icon.name: "documentation"
                text: i18n("Dictionary")
                page: "qrc:/Dictionary.qml"
                pagePool: mainPagePool
            },
            Kirigami.PagePoolAction {
                icon.name: "help-about"
                text: i18n("About KAlgebra")
                page: "qrc:/About.qml"
                pagePool: mainPagePool
            }
        ]
    }

    pageStack.initialPage: mainPagePool.loadPage("qrc:/Console.qml")
}
