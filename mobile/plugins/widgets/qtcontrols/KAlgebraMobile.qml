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
import QtQuick.Controls 2.1
import org.kde.analitza 1.0
import org.kde.kalgebra.mobile 1.0

ApplicationWindow
{
    id: rootItem
    height: 600
    width: 600
    visible: true

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            ToolButton {
                text: "\u2630"
                onClicked: drawer.open()
            }
            Label {
                text: "KAlgebra"
            }
            Item {
                Layout.fillWidth: true
            }
        }
    }

    property string currentPath: pluginsModel.pluginPath(0)
    onCurrentPathChanged: {
        var component = Qt.createComponent(currentPath);
        if (component.status == Component.Error) {
            console.log("error", component.errorString());
            return;
        }

        try {
            stack.replace(component)
        } catch(e) {
            console.log("error", e)
        }
        drawer.position = 0
    }

    Drawer {
        id: drawer
        edge: Qt.LeftEdge
        width: parent.width - 56 * 1
        height: parent.height

        ColumnLayout {
            anchors.fill: parent
            Repeater {
                delegate: ItemDelegate {
                    Layout.fillWidth: true
                    text: title
                    highlighted: model.path == rootItem.currentPath
                    onClicked: {
                        rootItem.currentPath = model.path
                    }
                }
                model: PluginsModel {
                    id: pluginsModel
                }
            }

            Item {
                width: 5
                Layout.fillHeight: true
            }
        }
    }

    StackView {
        id: stack
        anchors.fill: parent
    }
}
