add_subdirectory(widgets)

set(QML_SRC Console.qml Plot2D.qml Plot3D.qml Tables.qml Dictionary.qml
    About.qml VariablesView.qml)
install(FILES ${QML_SRC}
    kalgebraabout.json kalgebraconsole.json kalgebraplot2d.json kalgebraplot3d.json
    kalgebratables.json kalgebravariables.json kalgebradictionary.json
    DESTINATION ${KDE_INSTALL_DATADIR}/kalgebramobile/plugins/
)
ADD_CUSTOM_TARGET(QML SOURCES ${QML_SRC})
