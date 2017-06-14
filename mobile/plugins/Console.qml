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
//         variables: app.variables
        mode: ConsoleModel.Evaluate
        onErrorMessage: {
            itemModel.insert(0, { result: error })
            input.selectAll()
            view.currentIndex = 0
        }
        onOperationSuccessfulString: {
            itemModel.insert(0, { result: expression + "=" + result })
            input.selectAll()
            view.currentIndex = 0
        }
    }

    FileDialog {
        id: fileDialog
        folder: shortcuts.home
        onAccepted: proceed()

        property var proceed
    }

    function proceedLoadScript() {
        consoleModel.loadScript(fileDialog.fileUrl)
    }

    contextualActions: [
        Action {
            text: i18n("Load Script...")
            onTriggered: {
                fileDialog.title = text
                fileDialog.proceed = page.proceedLoadScript
                var v = fileDialog.open()
                console.log("opened...", v)
            }
        },
        Action {
            text: i18n("Save Script...")
        },
        //TODO: Recent scripts
        Action {
            text: i18n("Export Log...")
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
    
    ExpressionInput {
        id: input
        focus: true
        
        Keys.onReturnPressed: {
            consoleModel.addOperation(text)
        }
        
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: page.dp*6
        }
    }
    
    SimpleListView {
        id: view
        model: itemModel
        role: "result"
        title: "Results"
        
        anchors {
            top: input.bottom
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
    }
}
