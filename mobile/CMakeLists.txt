add_subdirectory(plugins)

if(KF5I18n_FOUND)
    add_definitions(-DKDECOMPONENTS)
endif()

qt5_add_resources(KALGEBRAMOBILE_SRCS resources.qrc)
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
    target_link_libraries(kalgebramobile Qt5::Svg)
elseif(KF5I18n_FOUND)
    target_link_libraries(kalgebramobile KF5::I18n)
endif()

set(DESKTOPFILE kalgebramobile.desktop)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/kalgebramobile${MOBILE_BACKEND}.desktop)
    set(DESKTOPFILE kalgebramobile${MOBILE_BACKEND}.desktop)
endif()

install(TARGETS kalgebramobile ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS ${DESKTOPFILE} DESTINATION ${DESKTOPFILE_INSTALL} )
