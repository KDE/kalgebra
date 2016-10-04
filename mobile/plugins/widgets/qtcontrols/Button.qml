import QtQuick.Controls 2.0

Button {
    readonly property var iconDictionary: {
        "list-add": "+"
    }

    property var iconName: "" //TODO
    text: iconDictionary[iconName]
}
