add_subdirectory(tests)

set(kalgebra_SRCS
	expression.cpp
	algebra.cpp
	analitza.cpp
	console.cpp
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
)

kde4_automoc( ${kalgebra_SRCS} )
kde4_add_executable(kalgebra ${kalgebra_SRCS} )
target_link_libraries(kalgebra ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${QT_QTOPENGL_LIBRARY})

install(TARGETS kalgebra DESTINATION ${BIN_INSTALL_DIR})
install(FILES kalgebra.desktop DESTINATION ${XDG_APPS_DIR} )

