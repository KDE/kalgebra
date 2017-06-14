install( FILES
    SimpleListView.qml Dialog.qml AddButton.qml Action.qml
    
    KAlgebraPage.qml KAlgebraMobile.qml
    DESTINATION ${KDE_INSTALL_DATADIR}/kalgebramobile/plugins/widgets/
)

install(FILES
    ../qtcontrols/ComboBox.qml ../qtcontrols/Label.qml
    ../qtcontrols/Button.qml ../qtcontrols/CalcButton.qml ../qtcontrols/RealInput.qml ../qtcontrols/ExpressionInput.qml

    DESTINATION ${KDE_INSTALL_DATADIR}/kalgebramobile/plugins/widgets/
)
