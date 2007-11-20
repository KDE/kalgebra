project(kalgebra)

#Checking whether we can compile calgebra
include(CheckIncludeFiles)
macro_optional_find_package(Readline)
macro_bool_to_01(READLINE_FOUND HAVE_READLINE)
macro_log_feature(READLINE_FOUND "GNU Readline" "Readline is needed by KAlgebra for the console version's interface"
	"http://tiswww.case.edu/php/chet/readline/rltop.html" FALSE)

macro_optional_find_package(OpenGL)
macro_bool_to_01(OPENGL_FOUND HAVE_OPENGL)
macro_log_feature(OPENGL_FOUND "OpenGL" "OpenGL is needed by KAlgebra, to have the 3D Graphs" "http://www.opengl.org/" FALSE)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/src/config.h)

#adding subdirs
add_subdirectory(src)
add_subdirectory(icons)

