import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Controls 2.0
import org.kde.analitza 1.0
import org.kde.kalgebra.mobile 1.0

TextField
{
    id: field

    readonly property string currentWord: field.selectionStart === field.selectionEnd ? operators.lastWord(field.cursorPosition, field.text) : ""
    onCurrentWordChanged: view.currentIndex = -1

    Window {
        height: 100
        width: field.width
        flags: Qt.ToolTip
        ListView {
            id: view
            anchors {
                fill: parent
                margins: 1
            }
            currentIndex: -1
            model: QSortFilterProxyModel {
                sourceModel: OperatorsModel { id: operators }
                filterRegExp: new RegExp("^" + field.currentWord)
            }

            delegate: ItemDelegate {
                text: model.display + " - " + description
                highlighted: view.currentIndex === index

                function complete() {
                    var toInsert = model.display.substr(field.currentWord.length);
                    if(isVariable)
                        toInsert += '(';
                    field.insert(field.cursorPosition, toInsert)
                }

                onClicked: complete()
                Keys.onReturnPressed: complete()
            }
        }

        visible: view.count >= 0 && field.activeFocus && field.currentWord.length > 0
        readonly property point globalPos: visible ? field.mapToGlobal(field.x, field.y) : Qt.point(0,0)

        x: globalPos.x
        y: globalPos.y + field.height
    }

    placeholderText: "Expression to calculate..."
    inputMethodHints: /*Qt.ImhPreferNumbers |*/ Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase

    Keys.forwardTo: view.visible && view.currentItem ? [ view.currentItem ] : null
    Keys.onUpPressed: view.decrementCurrentIndex()
    Keys.onDownPressed: view.incrementCurrentIndex()
    Keys.onReturnPressed: view.currentIndex = -1
}
