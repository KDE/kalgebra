/*
 *   Copyright 2012 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick
import org.kde.plasma.plasmoid
import org.kde.plasma.components as PlasmaComponents
import org.kde.kirigami as Kirigami
import org.kde.analitza

PlasmoidItem {
    width: Kirigami.Units.gridUnit * 15
    height: Kirigami.Units.gridUnit * 15

    property Component compactRepresentation: Component {
        PlasmaComponents.Button {
            icon.source: "kalgebra"
            onClicked: plasmoid.togglePopup()
        }
    }
    property string displayedFunction
    onDisplayedFunctionChanged: {
        plots.clear()
        view.addFunction(displayedFunction)
    }
    
    PlasmaComponents.TextField {
        id: input
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        onAccepted: {
            displayedFunction = text
            plasmoid.writeConfig("function", text)
        }
    }
    
    Component.onCompleted: {
        plasmoid.addEventListener('ConfigChanged', function() {
            displayedFunction = plasmoid.readConfig("function")
            input.text = displayedFunction
        });
    }
    
    Graph2D {
        id: view
        anchors {
            fill: parent
            topMargin: input.height
        }
        
        model: PlotsModel { id: plots }
        
        ticksShown: false
    }
}
