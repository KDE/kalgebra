set(QMLFILES AddButton.qml KAlgebraPage.qml KAlgebraMobile.qml
    ../qtcontrols/RealInput.qml ../qtcontrols/ExpressionInput.qml)

install( FILES
    AddButton.qml KAlgebraPage.qml KAlgebraMobile.qml
    DESTINATION ${KDE_INSTALL_DATADIR}/kalgebramobile/plugins/widgets/
)

install(FILES
    ../qtcontrols/RealInput.qml ../qtcontrols/ExpressionInput.qml
    DESTINATION ${KDE_INSTALL_DATADIR}/kalgebramobile/plugins/widgets/
)

ADD_CUSTOM_TARGET(QMLFILES SOURCES ${QMLFILES})