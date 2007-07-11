add_subdirectory(tests)

set(kalgebra_SRCS
	expression.cpp
	algebra.cpp
	analitza.cpp
	#console.cpp
	consolehtml.cpp
	container.cpp
	function.cpp
	functionedit.cpp
	varedit.cpp
	main.cpp
	object.cpp
	operator.cpp
	graph3d.cpp
	algebrahighlighter.cpp
	exp.cpp
	expressionedit.cpp
	value.cpp
	variables.cpp
	graph2d.cpp
	operatorsmodel.cpp
	dictionary.cpp
)

kde4_add_executable(kalgebra ${kalgebra_SRCS} )
target_link_libraries(kalgebra  ${KDE4_KHTML_LIBS} ${QT_QTOPENGL_LIBRARY} ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY})

install(TARGETS kalgebra DESTINATION ${BIN_INSTALL_DIR})
install(FILES kalgebra.desktop DESTINATION ${XDG_APPS_DIR} )

