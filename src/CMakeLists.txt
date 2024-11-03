qt_add_resources(KALGEBRA_SRCS resources.qrc)

add_executable(kalgebra
    ../src/consolemodel.cpp
    clipboard.cpp
    kalgebraapp.cpp
    main.cpp
    ${KALGEBRA_SRCS})

target_link_libraries(kalgebra
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
    find_package(KF6Kirigami REQUIRED)
    kirigami_package_breeze_icons(ICONS list-add)
    target_link_libraries(kalgebra PRIVATE Qt::Svg KF6::Kirigami Qt::QuickControls2)
else()
    find_package(Qt6Widgets REQUIRED)
    target_link_libraries(kalgebra PRIVATE Qt::Widgets)
endif()

install(TARGETS kalgebra ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
