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
import org.kde.kirigami 2.0 as Kirigami
import org.kde.analitza 1.0
import org.kde.kalgebra.mobile 1.0

Kirigami.ApplicationWindow
{
    id: rootItem
    height: 500
    width: 900
    visible: true

    contextDrawer: Kirigami.ContextDrawer {}

    globalDrawer: Kirigami.GlobalDrawer {
        id: drawer

        modal: Kirigami.Settings.isMobile
        handleVisible: modal

        title: "KAlgebra"
        titleIcon: "qrc:/kalgebra.svgz"
        bannerImageSource: "qrc:/header.jpg"
        modal: Kirigami.Settings.isMobile
        drawerOpen: true

        Instantiator {
            delegate: Kirigami.Action {
                text: title
                iconName: decoration
                onTriggered: {
                    var component = Qt.createComponent(model.path);
                    if (component.status == Component.Error) {
                        console.log("error", component.errorString());
                        return;
                    }

                    try {
                        rootItem.pageStack.replace(component, {title: title})
                    } catch(e) {
                        console.log("error", e)
                    }
                }
            }
            model: PluginsModel {}
            onObjectAdded: {
                var acts = [];
                for(var v in drawer.actions) {
                    acts.push(drawer.actions[v]);
                }
                acts.splice(index, 0, object)
                drawer.actions = acts;
            }
            onObjectRemoved: {
                var acts = [];
                for(var v in drawer.actions) {
                    acts.push(drawer.actions[v]);
                }
                drawer.actions.splice(drawer.actions.indexOf(object), 1)
                drawer.actions = acts;
            }
        }

        actions: []
    }

    Component.onCompleted: {
        drawer.actions[0].trigger()
    }
}
