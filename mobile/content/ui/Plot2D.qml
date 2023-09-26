// SPDX-FileCopyrightText: 2015 by Aleix Pol <aleixpol@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Dialogs
import QtQuick.Controls
import org.kde.kirigami as Kirigami
import org.kde.analitza
import org.kde.kalgebra.mobile

KAlgebraPage {
    id: page

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0


    FileDialog {
        id: fileDialog
        currentFolder: StandardPaths.standardLocations(StandardPaths.HomeLocation)[0]
        onAccepted: proceed()

        property var proceed
    }

    actions: [
        Kirigami.Action {
            icon.name: 'list-add'
            text: i18n('Add Plot')
            onTriggered: plotDialog.open()
        },
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