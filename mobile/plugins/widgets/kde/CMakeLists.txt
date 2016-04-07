install( FILES
    Keyboard.qml SimpleListView.qml
    
    KAlgebraPage.qml KAlgebraMobile.qml
    DESTINATION ${KDE_INSTALL_DATADIR}/kalgebramobile/plugins/widgets/
)

install(FILES
    ../qtcontrols/AddButton.qml ../qtcontrols/ComboBox.qml ../qtcontrols/Dialog.qml ../qtcontrols/Label.qml
    ../qtcontrols/Button.qml ../qtcontrols/CalcButton.qml ../qtcontrols/RealInput.qml ../qtcontrols/ExpressionInput.qml

    DESTINATION ${KDE_INSTALL_DATADIR}/kalgebramobile/plugins/widgets/
)
