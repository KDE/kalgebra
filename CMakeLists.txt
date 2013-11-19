project(kalgebra)
cmake_minimum_required(VERSION 2.8.11)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

find_package(Qt5 5.2 REQUIRED NO_MODULE COMPONENTS Widgets Qml Quick Xml Svg OpenGL PrintSupport Test)
find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

find_package(Analitza REQUIRED)

set(MOBILE_BACKEND "kde" CACHE STRING "Backend to install, currently. Check /mobile/plugins/widgets/*")
if(${MOBILE_BACKEND} STREQUAL "kde")
    find_package(KGuiAddons REQUIRED)
    find_package(KI18n REQUIRED)
    find_package(KConfigWidgets REQUIRED)
    find_package(KDBusAddons REQUIRED)
    find_package(KWindowSystem REQUIRED)
    find_package(KService REQUIRED)
    find_package(KCompletion REQUIRED)
    find_package(ItemViews REQUIRED)
    find_package(KIconThemes REQUIRED)
    find_package(KTextWidgets REQUIRED)
    find_package(XmlGui REQUIRED)
    find_package(KBookmarks REQUIRED)

	find_package(KDeclarative REQUIRED)
endif()
# macro_optional_find_package(LibKdeEdu)
# find_library(QTMML_LIBRARY qtmmlwidget)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}
	${CMAKE_INSTALL_PREFIX}/include/KDE #FIXME
)

remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
remove_definitions(-DQT_NO_KEYWORDS)

find_package(OpenGL)
set(HAVE_OPENGL ${OPENGL_FOUND})
# macro_log_feature(OPENGL_FOUND "OpenGL" "Support for 3D graphs in KAlgebra" "http://mesa3d.sourceforge.net" FALSE "" "")
# macro_log_feature(QTMML_LIBRARY "QtMathML Widget" "Supports displaying some formulas in a mathematic form" "http://doc.qt.nokia.com/solutions/4/qtmmlwidget/qtmmlwidget.html" FALSE "" "")

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config-kalgebra.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/src/config-kalgebra.h)
add_definitions (-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

if(KDocTools_FOUND)
  add_subdirectory(doc)
endif()
if(${MOBILE_BACKEND} STREQUAL "kde")
    add_subdirectory(src)
    add_subdirectory(plasmoids)
    add_subdirectory(icons)
    add_subdirectory(utils)
endif()
add_subdirectory(mobile)

add_custom_target(commandsdoc
	${CMAKE_CURRENT_BINARY_DIR}/utils/docbook_analitzacommands commands.docbook
	DEPENDS docbook_analitzacommands
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/doc/
	COMMENT "Generating commands docbook information"
)

# if (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
#    macro_display_feature_log()
# endif (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})

