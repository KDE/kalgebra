import QtQuick 1.1
import com.nokia.meego 1.0

ListView
{
    id: view
    property string role: ""
    delegate: Label { text: model[role] }
    
    ScrollDecorator {
        flickableItem: view
    }
}
