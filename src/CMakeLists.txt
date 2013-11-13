find_package(KI18n ${KF5_VERSION} REQUIRED)
find_package(KGuiAddons ${KF5_VERSION} REQUIRED)
find_package(KConfigWidgets ${KF5_VERSION} REQUIRED)
find_package(KDBusAddons ${KF5_VERSION} REQUIRED)
find_package(KService ${KF5_VERSION} REQUIRED)
find_package(KCompletion ${KF5_VERSION} REQUIRED)
find_package(KWindowSystem ${KF5_VERSION} REQUIRED)
find_package(ItemViews ${KF5_VERSION} REQUIRED)
find_package(KWidgetsAddons ${KF5_VERSION} REQUIRED)
find_package(KIconThemes ${KF5_VERSION} REQUIRED)
find_package(KTextWidgets ${KF5_VERSION} REQUIRED)
find_package(XmlGui ${KF5_VERSION} REQUIRED)
find_package(Solid ${KF5_VERSION} REQUIRED)
find_package(KBookmarks ${KF5_VERSION} REQUIRED)

find_package(KConfig REQUIRED)
# find_package(KDocTools)
find_package(KI18n REQUIRED)
find_package(KIO REQUIRED)

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
target_link_libraries(kalgebra Qt5::Widgets Qt5::PrintSupport Qt5::WebKitWidgets
                               KF5::KI18n KF5::XmlGui KF5::KIOWidgets
                               ${QTMML_LIBRARY} ${ANALITZA_LIBRARY} ${ANALITZAGUI_LIBRARY} ${ANALITZAPLOT_LIBRARY})

if(OPENGL_FOUND)
   target_link_libraries(kalgebra Qt5::OpenGL ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY})
endif(OPENGL_FOUND)

install(TARGETS kalgebra ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS kalgebra.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install(FILES kalgebra.xml DESTINATION ${DATA_INSTALL_DIR}/katepart/syntax )
