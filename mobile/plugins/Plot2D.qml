import QtQuick 2.0
import QtQuick.Layouts 1.0
import org.kde.analitza 1.0
import widgets 1.0

KAlgebraPage
{
    anchors.margins: 0
    
    RowLayout {
        id: controls
        spacing: 10
        
        width: parent.width
        ExpressionInput {
            id: input
            Layout.fillWidth: true
            text: "sin x"
            focus: true
            Component.onCompleted: selectAll()
            
            Keys.onReturnPressed: {
                view.addFunction(input.text, app.variables)
                selectAll();
            }
        }
        
        Button {
            text: "Clear"
            
            onClicked: {
                app.functionsModel().clear()
                view.resetViewport()
                input.focus = true;
                selectAll();
            }
        }
    }
    
    
    Rectangle {
        width: parent.width
        anchors {
            fill: parent
            topMargin: controls.height
        }
        height: 200
        color: 'white'
    
        Graph2D {
            id: view
            anchors.fill: parent
            model: app.functionsModel()
        }
    }
}
