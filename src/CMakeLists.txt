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
	functionimpl.cpp
)

kde4_add_library(analitza ${analitza_SRCS})
target_link_libraries(analitza ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY} ${KDE4_KDECORE_LIBS})

#uncomment this if oxygen icons for kalgebra are available
#set(kalgebra_icons
#    ${CMAKE_INSTALL_PREFIX}/share/icons/oxygen/16x16/apps/kalgebra.png
#    ${CMAKE_INSTALL_PREFIX}/share/icons/oxygen/32x32/apps/kalgebra.png
#    ${CMAKE_INSTALL_PREFIX}/share/icons/oxygen/48x48/apps/kalgebra.png
#    ${CMAKE_INSTALL_PREFIX}/share/icons/oxygen/64x64/apps/kalgebra.png
#    ${CMAKE_INSTALL_PREFIX}/share/icons/oxygen/128x128/apps/kalgebra.png
#)
set(kalgebra_icons
    ${CMAKE_CURRENT_SOURCE_DIR}/kalgebra/icons/hi16-app-kalgebra.png
    ${CMAKE_CURRENT_SOURCE_DIR}/kalgebra/icons/hi32-app-kalgebra.png
    ${CMAKE_CURRENT_SOURCE_DIR}/kalgebra/icons/hi48-app-kalgebra.png
    ${CMAKE_CURRENT_SOURCE_DIR}/kalgebra/icons/hi64-app-kalgebra.png
    ${CMAKE_CURRENT_SOURCE_DIR}/kalgebra/icons/hi128-app-kalgebra.png
)
kde4_add_win32_app_icon(kalgebra kalgebra_SRCS ${kalgebra_icons})

kde4_add_executable(kalgebra ${kalgebra_SRCS})
target_link_libraries(kalgebra  ${KDE4_KHTML_LIBS} ${QT_QTOPENGL_LIBRARY} ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY} analitza)

install(TARGETS kalgebra DESTINATION ${BIN_INSTALL_DIR})
#install(TARGETS analitza DESTINATION ${LIB_INSTALL_DIR})
install(FILES kalgebra.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )

