add_executable(kalgebra)
target_sources(kalgebra PRIVATE
    askname.h
    consolehtml.cpp
    consolehtml.h
    consolemodel.cpp
    consolemodel.h
    dictionary.cpp
    dictionary.h
    functionedit.cpp
    functionedit.h
    kalgebra.cpp
    kalgebra.h
    main.cpp
    varedit.cpp
    varedit.h
    variablesdelegate.cpp
    variablesdelegate.h
    viewportwidget.cpp
    viewportwidget.h
)

file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../icons/*-apps-kalgebra.png")
ecm_add_app_icon(kalgebra ICONS ${ICONS_SRCS})

target_link_libraries(kalgebra Qt::Widgets Qt::PrintSupport Qt::WebEngineWidgets KF6::I18n
                               KF6::CoreAddons KF6::WidgetsAddons KF6::ConfigWidgets
                               KF6::XmlGui # HelpMenu
                               KF6::KIOCore
                               KF6::I18n
                               KF6::Analitza KF6::AnalitzaWidgets KF6::AnalitzaGui KF6::AnalitzaPlot Qt6::OpenGLWidgets)

install(TARGETS kalgebra ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS org.kde.kalgebra.desktop DESTINATION ${KDE_INSTALL_APPDIR} )
install(FILES kalgebra.xml DESTINATION ${KDE_INSTALL_DATADIR}/katepart5/syntax )
install(FILES org.kde.kalgebra.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

