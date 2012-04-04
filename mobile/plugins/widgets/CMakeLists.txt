add_subdirectory(${MOBILE_BACKEND})

install( FILES
    qmldir
    DESTINATION ${DATA_INSTALL_DIR}/kalgebramobile/plugins/widgets/
)
