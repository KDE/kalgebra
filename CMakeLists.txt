project(kalgebra)

#Checking whether we can compile calgebra
include(CheckIncludeFiles)
macro_optional_find_package(Readline)
macro_bool_to_01(READLINE_FOUND HAVE_READLINE)
macro_log_feature(READLINE_FOUND "GNU Readline" "OCaml is needed by KAlgebra, to have the console version"
	"http://tiswww.case.edu/php/chet/readline/rltop.html" FALSE)

#adding subdirs
add_subdirectory(src)
add_subdirectory(icons)

