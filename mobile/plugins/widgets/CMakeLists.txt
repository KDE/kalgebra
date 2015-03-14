add_subdirectory(${MOBILE_BACKEND})

install( FILES
    qmldir
    DESTINATION ${KDE_INSTALL_DATADIR}/kalgebramobile/plugins/widgets/
)
