add_subdirectory(plugins)

if(TARGET KF5::I18n)
    add_definitions(-DKDECOMPONENTS)
endif()

qt5_add_resources(KALGEBRAMOBILE_SRCS resources.qrc header.qrc)
add_executable(kalgebramobile pluginsmodel.cpp
    ../src/consolemodel.cpp
    kalgebramobile.cpp main.cpp ${KALGEBRAMOBILE_SRCS})

target_link_libraries(kalgebramobile Qt5::Qml Qt5::Quick Qt5::Gui
    KF5::Analitza KF5::AnalitzaGui KF5::AnalitzaPlot)

set(DESKTOPFILE_INSTALL ${KDE_INSTALL_APPDIR})

if(CMAKE_SYSTEM_NAME STREQUAL "Android")
#   Material requires QtSvg for icons
#   if we don't link it here explicitly, androiddeployqt doesn't bring it
    find_package(Qt5Svg REQUIRED)
    find_package(KF5Kirigami2 REQUIRED)
    target_link_libraries(kalgebramobile Qt5::Svg KF5::Kirigami2)
endif()

if(TARGET KF5::I18n)
    target_link_libraries(kalgebramobile KF5::I18n)
endif()

install(TARGETS kalgebramobile ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS org.kde.kalgebramobile.desktop DESTINATION ${DESKTOPFILE_INSTALL} )
install(FILES org.kde.kalgebramobile.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
