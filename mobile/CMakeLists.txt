qt_add_resources(KALGEBRAMOBILE_SRCS resources.qrc)

add_executable(kalgebramobile
    ../src/consolemodel.cpp
    clipboard.cpp
    kalgebramobile.cpp
    main.cpp
    ${KALGEBRAMOBILE_SRCS})

target_link_libraries(kalgebramobile
    PRIVATE
        Qt6::Qml
        Qt6::Quick
        Qt6::Gui
        KF6::CoreAddons
        KF6::I18n
        KF6::Analitza
        KF6::AnalitzaGui
        KF6::AnalitzaPlot
)

set(DESKTOPFILE_INSTALL ${KDE_INSTALL_APPDIR})

if(ANDROID)
#   Material requires QtSvg for icons
#   if we don't link it here explicitly, androiddeployqt doesn't bring it
    find_package(Qt6Svg REQUIRED)
    find_package(Qt6QuickControls2 REQUIRED)
    find_package(KF5Kirigami2 REQUIRED)
    kirigami_package_breeze_icons(ICONS list-add)
    target_link_libraries(kalgebramobile PRIVATE Qt::Svg KF5::Kirigami2 Qt::QuickControls2)
else()
    find_package(Qt6Widgets REQUIRED)
    target_link_libraries(kalgebramobile PRIVATE Qt::Widgets)
endif()

install(TARGETS kalgebramobile ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS org.kde.kalgebramobile.desktop DESTINATION ${DESKTOPFILE_INSTALL} )
install(FILES org.kde.kalgebramobile.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
