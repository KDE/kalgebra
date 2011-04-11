project(kalgebra)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)
set(CMAKE_MODULE_PATH ${kalgebra_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

find_package(LibKdeEdu REQUIRED)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${LIBKDEEDU_INCLUDE_DIR}/qtmml)

#Checking whether we can compile calgebra
include(CheckIncludeFiles)
macro_optional_find_package(Readline)
macro_bool_to_01(READLINE_FOUND HAVE_READLINE)
macro_log_feature(READLINE_FOUND "GNU Readline" "A library for editing typed command lines" "http://cnswww.cns.cwru.edu/php/chet/readline/rltop.html" FALSE "" "Gives KAlgebra a console interface.")

macro_optional_find_package(OpenGL)
macro_bool_to_01(OPENGL_FOUND HAVE_OPENGL)
macro_log_feature(OPENGL_FOUND "OpenGL" "API for developing portable, interactive 2D and 3D graphics applications" "http://mesa3d.sourceforge.net" FALSE "" "Provides 3D graphs in KAlgebra.")

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config-kalgebra.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/src/config-kalgebra.h)
option(INSTALL_ANALITZA_HEADERS "Analitza headers will be installed. No ABI compatibility is going to happen for now" OFF)

include_directories(.)

add_subdirectory(doc)
add_subdirectory(analitza)
add_subdirectory(analitzagui)
add_subdirectory(src)
add_subdirectory(icons)
add_subdirectory(plasmoid)
add_subdirectory(utils)
if(NOT MSVC)
  add_subdirectory(mobile)
endif(NOT MSVC)
if(HAVE_READLINE)
	add_subdirectory(calgebra)
endif(HAVE_READLINE)

add_custom_target(commandsdoc
	${CMAKE_CURRENT_BINARY_DIR}/utils/docbook_analitzacommands commands.docbook
	DEPENDS docbook_analitzacommands
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/doc/kalgebra/
	COMMENT "Generating commands docbook information"
)

