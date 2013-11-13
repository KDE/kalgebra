add_subdirectory(widgets)

install( FILES
	Console.qml Plot2D.qml Tables.qml
	About.qml VariablesView.qml
	kalgebraabout.json
	kalgebraconsole.json kalgebraplot2d.json kalgebratables.json kalgebravariables.json
	DESTINATION ${DATA_INSTALL_DIR}/kalgebramobile/plugins/
)
