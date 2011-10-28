project(kalgebra)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)
set(CMAKE_MODULE_PATH ${kalgebra_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

find_package(Analitza REQUIRED)
find_package(LibKdeEdu)
find_library(QTMML_LIBRARY qtmmlwidget)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${LIBKDEEDU_INCLUDE_DIR}/qtmml)

macro_optional_find_package(OpenGL)
macro_bool_to_01(OPENGL_FOUND HAVE_OPENGL)
macro_log_feature(OPENGL_FOUND "OpenGL" "Support for 3D graphs in KAlgebra" "http://mesa3d.sourceforge.net" FALSE "" "")

macro_log_feature(QTMML_LIBRARY "QtMathML Widget" "Supports displaying some formulas in a mathematic form" "http://doc.qt.nokia.com/solutions/4/qtmmlwidget/qtmmlwidget.html" FALSE "" "")

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config-kalgebra.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/src/config-kalgebra.h)

include_directories(.)
add_definitions (-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

add_subdirectory(doc)
if(QTMML_LIBRARY)
  add_subdirectory(src)
endif(QTMML_LIBRARY)
add_subdirectory(icons)
add_subdirectory(plasmoid)
add_subdirectory(utils)
add_subdirectory(mobile)
if(HAVE_READLINE)
	add_subdirectory(calgebra)
endif(HAVE_READLINE)

add_custom_target(commandsdoc
	${CMAKE_CURRENT_BINARY_DIR}/utils/docbook_analitzacommands commands.docbook
	DEPENDS docbook_analitzacommands
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/doc/
	COMMENT "Generating commands docbook information"
)

if (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
   macro_display_feature_log()
endif (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})

