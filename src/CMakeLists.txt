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

#uncomment this if oxygen icons for kalgebra are available
#kde4_add_app_icon(kalgebra_SRCS "${CMAKE_INSTALL_PREFIX}/share/icons/oxygen/*/apps/kalgebra.png")
# kde4_add_app_icon(kalgebra_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../icons/hi*-app-kalgebra.png") #FIXME: port!

add_executable(kalgebra ${kalgebra_SRCS})
target_link_libraries(kalgebra Qt5::Widgets Qt5::PrintSupport Qt5::WebKitWidgets KF5::I18n
                               KF5::CoreAddons KF5::WidgetsAddons KF5::ConfigWidgets
                               KF5::XmlGui # HelpMenu
                               KF5::KIOCore
                               KF5::I18n
                               KF5::Analitza KF5::AnalitzaWidgets KF5::AnalitzaGui KF5::AnalitzaPlot)

if(OPENGL_FOUND)
   target_link_libraries(kalgebra Qt5::OpenGL ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY})
endif(OPENGL_FOUND)

install(TARGETS kalgebra ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS org.kde.kalgebra.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install(FILES kalgebra.xml DESTINATION ${DATA_INSTALL_DIR}/katepart5/syntax )


