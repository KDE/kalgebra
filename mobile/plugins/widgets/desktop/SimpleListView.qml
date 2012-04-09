import QtDesktop 0.1

TableView
{
    id: view
    property alias role: col.role
    property alias title: col.title
    headerVisible: title!=""

    TableColumn {
        id: col
        
        title: "Value"
		width: parent.width-32
    }
}