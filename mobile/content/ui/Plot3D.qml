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

Kirigami.Page {
    id: page

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    title: i18nc("@title:window", "3D Plot")

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
            icon.name: 'document-save'
            onTriggered: {
                fileDialog.title = text
                fileDialog.proceed = function() {
                    var ret = view.save(fileDialog.fileUrl)
                    console.log("saved 3D", fileDialog.fileUrl, ret)
                }
                fileDialog.nameFilters = view.filters
                fileDialog.selectExisting = false
                fileDialog.open()
            }
        },
        Kirigami.Action {
            icon.name: 'view-restore'
            text: i18n("Reset Viewport")
            onTriggered: view.resetViewport()
        }
    ]


    Graph3D {
        id: view
        anchors.fill: parent
        model: App.functionsModel()
        AddPlotDialog {
            id: plotDialog
            text: "sin x*sin y"
            dimension: 4
        }
    }
}
