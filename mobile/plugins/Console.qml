import QtQuick 2.2
import QtQuick.Dialogs 1.0
import org.kde.analitza 1.0
import org.kde.kalgebra.mobile 1.0
import widgets 1.0

KAlgebraPage
{
    id: page
    ListModel { id: itemModel }

    ConsoleModel {
        id: consoleModel
        variables: app.variables
        onMessage: {
            itemModel.append({ result: msg })
            input.selectAll()
            view.currentIndex = view.count-1
            view.positionViewAtIndex(view.currentIndex, ListView.Contain)
        }
    }

    FileDialog {
        id: fileDialog
        folder: shortcuts.home
        onAccepted: proceed()

        property var proceed
    }

    contextualActions: [
        Action {
            text: i18n("Load Script...")
            onTriggered: {
                fileDialog.title = text
                fileDialog.proceed = function() { consoleModel.loadScript(fileDialog.fileUrl) }
                fileDialog.nameFilters = [ i18n("Script (*.kal)") ]
                fileDialog.selectExisting = true
                fileDialog.open()
            }
        },
        Action {
            text: i18n("Save Script...")
            onTriggered: {
                fileDialog.title = text
                fileDialog.proceed = function() { consoleModel.saveScript(fileDialog.fileUrl) }
                fileDialog.nameFilters = [ i18n("Script (*.kal)") ]
                fileDialog.selectExisting = false
                fileDialog.open()
            }
        },
        //TODO: Recent scripts
        Action {
            text: i18n("Export Log...")
            onTriggered: {
                fileDialog.title = text
                fileDialog.proceed = function() { consoleModel.saveLog(fileDialog.fileUrl) }
                fileDialog.nameFilters = [ i18n("HTML (*.html)") ]
                fileDialog.selectExisting = false
                fileDialog.open()
            }
        },
        // --
        Action {
            text: consoleModel.mode == ConsoleModel.Calculate ? i18n("Evaluate...") : i18n("Calculate...")
            onTriggered: consoleModel.mode = !consoleModel.mode
        },
        // --
        Action {
            iconName: "edit-clear-history"
            text: i18n("Clear Log")
            onTriggered: itemModel.clear()
        }
    ]
    
    SimpleListView {
        id: view
        model: itemModel
        role: "result"

        anchors {
            top: parent.top
            bottom: input.top
            left: parent.left
            right: parent.right
        }
    }

    ExpressionInput {
        id: input
        focus: true
        
        Keys.onReturnPressed: {
            consoleModel.addOperation(text)
        }
        
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
    }
}
