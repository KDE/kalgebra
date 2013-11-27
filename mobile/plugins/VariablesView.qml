import QtQuick 2.0
import org.kde.analitza 1.0
import widgets 1.0

KAlgebraPage
{
	SimpleListView {
		anchors.fill: parent
		model: VariablesModel { variables: app.variables }
		currentIndex: -1
		role: "whatsThis"
		title: "Variables"
	}
}
