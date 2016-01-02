import QtQuick 2.0
import QtQuick.Layouts 1.0
import QtQuick.Controls 1.4 as Controls
import org.kde.analitza 1.0
import widgets 1.0

KAlgebraPage
{
    id: page
    anchors.margins: 0
    
    ColumnLayout {
        id: descriptioncol

        anchors.fill: parent
        spacing: 10

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
                    chosebox.updateGraph();
                }
            }
        }

        OperatorsModel {
            id: operators
        }
        Controls.ComboBox {
            anchors {
                left: parent.left
                right: parent.right
            }
            id: chosebox
            currentIndex: 29
            activeFocusOnPress: true

            model: ListModel {
                id: cbItems
                Component.onCompleted: {
                    for (var i=0; i<operators.rowCount(); i++)
                        cbItems.append( { text: operators.data(operators.index(i,0)) } );
                }
            }

            function updateGraph() {
                if(examplelabel!=null)
                {
                    app.functionsModel().clear();
                    view.resetViewport();
                    view.addFunction(examplelabel.text,app.variables);
                }
            }

            onCurrentIndexChanged: {
                updateGraph();
            }
        }

        GridLayout {
            id: descgrid
            columns: 2
            Layout.fillWidth: true

            Label { text: operators.headerData(0,Qt.Horizontal) }   Label { id: namelabel; text: operators.data(operators.index(chosebox.currentIndex,0)) }
            Label { text: operators.headerData(1,Qt.Horizontal) }   Label { id: desclabel; text: operators.data(operators.index(chosebox.currentIndex,1)) }
            Label { text: operators.headerData(2,Qt.Horizontal) }   Label { id: paramslabel; text: operators.data(operators.index(chosebox.currentIndex,2)) }
            Label { text: operators.headerData(3,Qt.Horizontal) }   Label { id: examplelabel; text: operators.data(operators.index(chosebox.currentIndex,3)) }
        }
    }
}
