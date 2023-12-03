// SPDX-FileCopyrightText: 2015 by Aleix Pol <aleixpol@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Dialogs
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.analitza
import org.kde.kalgebra.mobile

Kirigami.Page {
    id: page

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    title: i18nc("@title:window", "2D Plot")

    FileDialog {
        id: fileDialog
        currentFolder: StandardPaths.standardLocations(StandardPaths.HomeLocation)[0]
        onAccepted: proceed()

        property var proceed
    }

    actions: [
        QQC2.Action {
            icon.name: 'list-add'
            text: i18n('Add Plot')
            onTriggered: plotDialog.open()
        },
        QQC2.Action {
            text: i18n("Save")
            icon.name: 'document-save'
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
        QQC2.Action {
            text: i18n("View Grid")
            icon.name: 'view-grid'
            checkable: true
            checked: view.showGrid
            onToggled: view.showGrid = checked
        },
        QQC2.Action {
            icon.name: 'view-restore'
            text: i18n("Reset Viewport")
            onTriggered: view.resetViewport()
        }
        //custom viewport?
    ]

    Rectangle {
        anchors.fill: parent
        color: 'white'

        Graph2D {
            id: view
            anchors.fill: parent
            model: App.functionsModel()
            AddPlotDialog {
                id: plotDialog
                text: "sin x"
                dimension: 2
            }
        }
    }
}