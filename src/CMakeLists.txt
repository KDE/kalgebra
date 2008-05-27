include_directories(../analitza)

add_subdirectory(tests)

set(kalgebra_SRCS
	algebrahighlighter.cpp
	kalgebra.cpp
	dictionary.cpp
	#console.cpp
	consolehtml.cpp
	function.cpp
	functionedit.cpp
	varedit.cpp
	main.cpp
	graph3d.cpp
	expressionedit.cpp
	graph2d.cpp
	operatorsmodel.cpp
	variablesmodel.cpp
	functionsview.cpp
	functionimpl.cpp
	functionsmodel.cpp
)

#uncomment this if oxygen icons for kalgebra are available
#kde4_add_app_icon(kalgebra_SRCS "${CMAKE_INSTALL_PREFIX}/share/icons/oxygen/*/apps/kalgebra.png")
kde4_add_app_icon(kalgebra_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../icons/hi*-app-kalgebra.png")

kde4_add_executable(kalgebra ${kalgebra_SRCS})
target_link_libraries(kalgebra ${KDE4_KHTML_LIBS} analitza)

if(OPENGL_FOUND)
   target_link_libraries(kalgebra ${QT_QTOPENGL_LIBRARY} ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY})
endif(OPENGL_FOUND)

install(TARGETS kalgebra ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES kalgebra.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
