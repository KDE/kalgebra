project(kalgebra)
cmake_minimum_required(VERSION 2.8.11)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${ECM_MODULE_PATH})

find_package(Qt5 5.2 REQUIRED NO_MODULE COMPONENTS Qml Quick Xml Svg OpenGL PrintSupport Test)
include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)
include(ECMInstallIcons)
include(FeatureSummary)

find_package(Analitza5 REQUIRED)

set(MOBILE_BACKEND "kde" CACHE STRING "Backend to install, currently. Check /mobile/plugins/widgets/*")

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

find_package(OpenGL)
set(HAVE_OPENGL ${OPENGL_FOUND})
set_package_properties(OpenGL PROPERTIES TYPE OPTIONAL
                                         PURPOSE "Support for 3D graphs in KAlgebra"
                                         URL "http://opengl.org")

set(CURSES_NEED_NCURSES TRUE)
find_package(Curses)
find_package(Readline)

set_package_properties(Readline PROPERTIES TYPE OPTIONAL
                                           PURPOSE "Allows KAlgebra to provide a console interface."
                                           URL "http://cnswww.cns.cwru.edu/php/chet/readline/rltop.html")
set_package_properties(Curses PROPERTIES TYPE OPTIONAL
                                         PURPOSE "Allows KAlgebra to provide a console interface."
                                         URL "http://www.gnu.org/software/ncurses/")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-kalgebra.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/src/config-kalgebra.h)
add_definitions (-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

if(${MOBILE_BACKEND} STREQUAL "kde")
    find_package(KF5I18n ${KF5_VERSION} REQUIRED)
    find_package(KF5ConfigWidgets ${KF5_VERSION} REQUIRED)
    find_package(KF5WidgetsAddons ${KF5_VERSION} REQUIRED)
    find_package(KF5KIO REQUIRED)
    find_package(KF5DocTools REQUIRED)
    find_package(Qt5WebKitWidgets REQUIRED)

    add_subdirectory(src)
    add_subdirectory(plasmoids)
endif()

add_subdirectory(icons)
add_subdirectory(mobile)

if(READLINE_FOUND AND CURSES_FOUND)
    add_subdirectory(calgebra)
endif()

if(KF5DocTools_FOUND)
  add_subdirectory(utils)

  add_custom_target(commandsdoc
    ${CMAKE_CURRENT_BINARY_DIR}/utils/docbook_analitzacommands commands.docbook
    DEPENDS docbook_analitzacommands
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/doc/
    COMMENT "Generating commands docbook information"
  )

  add_subdirectory(doc)
endif()
install(FILES kalgebra.appdata.xml DESTINATION ${SHARE_INSTALL_PREFIX}/appdata/)

feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
