// SPDX-FileCopyrightText: 2015 by Aleix Pol <aleixpol@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.0
import QtQuick.Layouts 1.0
import QtQuick.Dialogs 1.0
import QtQuick.Controls 2.3
import org.kde.kirigami 2.5 as Kirigami
import org.kde.analitza 1.0
import org.kde.kalgebra.mobile 1.0

KAlgebraPage {
    id: page

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0


    FileDialog {
        id: fileDialog
        folder: shortcuts.home
        onAccepted: proceed()

        property var proceed
    }

    contextualActions: [
        Kirigami.Action {
            text: i18n("Save")
            onTriggered: {
                fileDialog.title = text
                fileDialog.proceed = function() {
                    var ret = view.save(fileDialog.fileUrl)
                    console.log("saved 2D", fileDialog.fileUrl, ret)
                }
                fileDialog.nameFilters = view.filters
                fileDialog.selectExisting = false
                fileDialog.open()
            }
        },
        Kirigami.Action {
            text: i18n("View Grid")
            checkable: true
            checked: view.showGrid
            onToggled: view.showGrid = checked
        },
        Kirigami.Action {
            text: i18n("Reset Viewport")
            onTriggered: view.resetViewport()
        }
        //custom viewport?
    ]

    actions.main: Kirigami.Action {
        icon.name: 'list-add'
        text: i18n('Add Plot')
        onTriggered: plotDialog.open()
    }
    
    Rectangle {
        anchors.fill: parent
        height: 200
        color: 'white'

        Graph2D {
            id: view
            anchors.fill: parent
            model: App.functionsModel()
            Add2DDialog {
                id: plotDialog
            }
        }
    }
}