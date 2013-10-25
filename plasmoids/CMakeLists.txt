# add_subdirectory(calculator)
# TODO: port to QML

install(DIRECTORY graphsplasmoid/ DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/org.kde.graphsplasmoid)
install(FILES graphsplasmoid/metadata.desktop DESTINATION ${SERVICES_INSTALL_DIR} RENAME graphsplasmoid.desktop)

