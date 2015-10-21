add_subdirectory(widgets)

install( FILES
    Console.qml Plot2D.qml Plot3D.qml Tables.qml
    About.qml VariablesView.qml
    kalgebraabout.json
    kalgebraconsole.json kalgebraplot2d.json kalgebraplot3d.json kalgebratables.json kalgebravariables.json
    DESTINATION ${KDE_INSTALL_DATADIR}/kalgebramobile/plugins/
)
