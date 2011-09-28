add_subdirectory(widgets)

install( FILES
	Console.qml Plot2D.qml Tables.qml
	kalgebraconsole.desktop kalgebraplot2d.desktop kalgebratables.desktop
	DESTINATION ${DATA_INSTALL_DIR}/kalgebra/plugins/
)
