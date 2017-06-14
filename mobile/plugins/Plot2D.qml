import QtQuick 2.0
import QtQuick.Layouts 1.0
import org.kde.analitza 1.0
import widgets 1.0

KAlgebraPage
{
    id: page
    anchors.margins: 0

    contextualActions: [
        Action {
            text: i18n("View Grid")
            checkable: true
            checked: view.showGrid
            onToggled: view.showGrid = checked
        },
        Action {
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
            model: app.functionsModel()

            Dialog {
                id: dialog
                height: Math.min((4*view.height)/5, list.contentHeight)+page.dp*10

                SimpleListView {
                    id: list
                    anchors {
                        fill: parent
                        margins: page.dp*4
                    }
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
                            onAccepted: {
                                input.selectAll()
                                view.addFunction(input.text, app.variables)
                            }
                        }
                        Button {
                            iconName: "list-add"
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
