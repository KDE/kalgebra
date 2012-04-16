import QtQuick 1.0
import org.kde.analitza 1.0
import widgets 1.0

KAlgebraPage
{
	//TODO: Port to simple list view
	ListView {
		anchors.fill: parent
		model: VariablesModel { variables: app.variables }
		delegate: Label { text: display+": "+toolTip }
	}
}