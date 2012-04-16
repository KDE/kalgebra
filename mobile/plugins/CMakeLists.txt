add_subdirectory(widgets)

install( FILES
	Console.qml Plot2D.qml Tables.qml
	Graph2D.qml About.qml VariablesView.qml
	kalgebraconsole.desktop kalgebraplot2d.desktop kalgebratables.desktop kalgebraabout.desktop kalgebravariables.desktop
	DESTINATION ${DATA_INSTALL_DIR}/kalgebramobile/plugins/
)
