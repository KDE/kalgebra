# add_subdirectory(calculator)
# TODO: port to QML

install(DIRECTORY graphsplasmoid/ DESTINATION ${KDE_INSTALL_DATADIR}/plasma/plasmoids/org.kde.graphsplasmoid)
install(FILES graphsplasmoid/metadata.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR} RENAME graphsplasmoid.desktop)

