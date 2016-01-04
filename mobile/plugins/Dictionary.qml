import QtQuick 2.0
import QtQuick.Layouts 1.0
import org.kde.analitza 1.0
import widgets 1.0

KAlgebraPage
{
    id: page
    anchors.margins: 0

    function updateGraph() {
        app.functionsModel().clear();
        view.resetViewport();
        view.addFunction(operators.data(operators.index(chosebox.currentIndex,3)), app.variables);
    }
    
    ColumnLayout {
        id: descriptioncol

        anchors.fill: parent
        spacing: 10

        ComboBox {
            id: chosebox
            Layout.fillWidth: true
            textRole: "display"

            model: OperatorsModel {
                id: operators
            }

            onCurrentIndexChanged: {
                page.updateGraph();
            }
        }

        GridLayout {
            id: descgrid
            columns: 2
            Layout.fillWidth: true

            Label { text: operators.headerData(0,Qt.Horizontal) }
            Label { text: operators.data(operators.index(chosebox.currentIndex,0)) }
            Label { text: operators.headerData(1,Qt.Horizontal) }
            Label { text: operators.data(operators.index(chosebox.currentIndex,1)) }
            Label { text: operators.headerData(2,Qt.Horizontal) }
            Label { text: operators.data(operators.index(chosebox.currentIndex,2)) }
            Label { text: operators.headerData(3,Qt.Horizontal) }
            Label { text: operators.data(operators.index(chosebox.currentIndex,3)) }
        }

        Rectangle {
            color: 'white'
            Layout.fillWidth: true
            Layout.fillHeight: true

            Graph2D {
                id: view
                anchors {
                    fill: parent
                }
                model: app.functionsModel()
                Component.onCompleted: {
                    page.updateGraph();
                }
            }
        }
    }
}
