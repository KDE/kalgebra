if(OPENGL_FOUND)
	include_directories(${OPENGL_INCLUDE_DIR})
endif(OPENGL_FOUND)

set(kalgebra_SRCS
	kalgebra.cpp
	dictionary.cpp
	#console.cpp
	consolehtml.cpp
	varedit.cpp
	main.cpp
	variablesdelegate.cpp
	viewportwidget.cpp
	functionedit.cpp
)

if(OPENGL_FOUND)
	set(kalgebra_SRCS
		${kalgebra_SRCS}
		graph3d.cpp
	)
endif(OPENGL_FOUND)

#uncomment this if oxygen icons for kalgebra are available
#kde4_add_app_icon(kalgebra_SRCS "${CMAKE_INSTALL_PREFIX}/share/icons/oxygen/*/apps/kalgebra.png")
kde4_add_app_icon(kalgebra_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../icons/hi*-app-kalgebra.png")

kde4_add_executable(kalgebra ${kalgebra_SRCS})
target_link_libraries(kalgebra ${KDE4_KHTML_LIBS} ${QTMML_LIBRARY} ${ANALITZA_LIBRARY} ${ANALITZAGUI_LIBRARY} ${ANALITZAPLOT_LIBRARY})

if(OPENGL_FOUND)
   target_link_libraries(kalgebra ${QT_QTOPENGL_LIBRARY} ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY} ${QGLVIEWER_LIBRARY})
endif(OPENGL_FOUND)

install(TARGETS kalgebra ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS kalgebra.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install(FILES kalgebra.xml DESTINATION ${DATA_INSTALL_DIR}/katepart/syntax )
