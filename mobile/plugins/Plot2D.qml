import QtQuick 2.0
import QtQuick.Layouts 1.0
import QtQuick.Controls 1.0 as Controls
import org.kde.analitza 1.0
import widgets 1.0

KAlgebraPage
{
    anchors.margins: 0
    
    Rectangle {
        anchors.fill: parent
        height: 200
        color: 'white'

        Graph2D {
            id: view
            anchors.fill: parent
            model: app.functionsModel()

            Dialog {
                id: dialog
                height: Math.min((4*view.height)/5, list.contentHeight)

                SimpleListView {
                    id: list
                    anchors.fill: parent
                    role: "description"
                    model: app.functionsModel()

                    header: RowLayout {
                        width: parent.width
                        ExpressionInput {
                            id: input
                            Layout.fillWidth: true
                            text: "sin x"
                            focus: true
                            Component.onCompleted: selectAll()
                        }
                        Button {
                            iconName: "content/add"
                            onClicked: {
                                input.selectAll()
                                view.addFunction(input.text, app.variables)
                            }
                        }
                    }

                    footer: Button {
                        text: "Clear All"
                        onClicked: {
                            app.functionsModel().clear();
                            view.resetViewport();
                        }
                    }
                }
            }

            AddButton {
                onClicked: {
                    dialog.open();
                }
            }
        }
    }
}
