import QtQuick 2.2
import QtQuick.Dialogs 1.0
import org.kde.analitza 1.0
import widgets 1.0

KAlgebraPage
{
    id: page
    ListModel { id: itemModel }

    Analitza {
        id: a
        variables: app.variables
        calculate: false
    }

    FileDialog {
        id: fileDialog
        folder: shortcuts.home
        onAccepted: proceed()

        property var proceed
    }

    function proceedLoadScript()

    contextualActions: [
        Action {
            text: i18n("Load Script...")
            onTriggered: {
                fileDialog.title = text
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
            text: a.calculate ? i18n("Evaluate...") : i18n("Calculate...")
            onTriggered: a.calculate = !a.calculate
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
            var res = a.execute(text)
            
            var toadd = ""
            if(!a.isCorrect)
                toadd = "Error: " + (res ? res : a.errors)
            else
                toadd = text + " = " + res.expression
            
            itemModel.insert(0, { result: toadd, resultsInput: text })
            input.selectAll()
            view.currentIndex = 0
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
