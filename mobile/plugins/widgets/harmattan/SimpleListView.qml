import QtQuick 1.1

ListView
{
    id: view
    property string role: ""
    delegate: Label { text: model[role] }
}
