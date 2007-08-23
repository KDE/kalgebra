add_subdirectory(tests)
if(HAVE_READLINE)
	macro_optional_add_subdirectory(calgebra)
endif(HAVE_READLINE)

include_directories(analitza)

set(analitza_SRCS
	analitza/expression.cpp
	analitza/analitza.cpp
	analitza/container.cpp
	analitza/operator.cpp
	analitza/exp.cpp
	analitza/variables.cpp
	analitza/object.cpp
	analitza/value.cpp
)

set(kalgebra_SRCS
	algebra.cpp
	#console.cpp
	consolehtml.cpp
	function.cpp
	functionedit.cpp
	varedit.cpp
	main.cpp
	graph3d.cpp
	algebrahighlighter.cpp
	expressionedit.cpp
	graph2d.cpp
	operatorsmodel.cpp
	dictionary.cpp
)

kde4_add_library(analitza ${analitza_SRCS})
target_link_libraries(analitza ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY} ${KDE4_KDECORE_LIBS})

kde4_add_executable(kalgebra ${kalgebra_SRCS})
target_link_libraries(kalgebra  ${KDE4_KHTML_LIBS} ${QT_QTOPENGL_LIBRARY} ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY} analitza)

install(TARGETS kalgebra DESTINATION ${BIN_INSTALL_DIR})
#install(TARGETS analitza DESTINATION ${LIB_INSTALL_DIR})
install(FILES kalgebra.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )

