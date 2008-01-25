project(kalgebra)

#Checking whether we can compile calgebra
include(CheckIncludeFiles)
macro_optional_find_package(Readline)
macro_bool_to_01(READLINE_FOUND HAVE_READLINE)
macro_log_feature(READLINE_FOUND "GNU Readline" "A library for editing typed command lines" "http://cnswww.cns.cwru.edu/php/chet/readline/rltop.html" FALSE "" "Gives KAlgebra a console interface.")

macro_optional_find_package(OpenGL)
macro_bool_to_01(OPENGL_FOUND HAVE_OPENGL)
macro_log_feature(OPENGL_FOUND "OpenGL" "API for developing portable, interactive 2D and 3D graphics applications" "http://mesa3d.sourceforge.net" FALSE "" "Provides 3D graphs in KAlgebra.")

macro_optional_find_package(Plasma)
macro_bool_to_01(PLASMA_FOUND HAVE_PLASMA)
macro_log_feature(PLASMA_FOUND "Plasma" "Plasma is the KDE4 desktop" "http://plasma.kde.org" FALSE "" "Provides a KAlgebra Plasmoid")

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config-kalgebra.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/src/config-kalgebra.h)

#adding subdirs
add_subdirectory(src)
add_subdirectory(icons)

