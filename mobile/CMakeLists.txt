add_subdirectory(plugins)

kde4_add_executable(kalgebramobile pluginsmodel.cpp graph2dmobile.cpp analitzawrapper.cpp
	kalgebramobile.cpp main.cpp console.cpp)

target_link_libraries(kalgebramobile ${KDE4_KDECORE_LIBS} ${QT_QTDECLARATIVE_LIBRARY}
	${QT_QTGUI_LIBRARY} analitza analitzagui)

set(DESKTOPFILE_INSTALL ${XDG_APPS_INSTALL_DIR})
if(${MOBILE_BACKEND} STREQUAL "harmattan")
	set(DESKTOPFILE_INSTALL ${XDG_APPS_INSTALL_DIR}/..)
endif(${MOBILE_BACKEND} STREQUAL "harmattan")

install(TARGETS kalgebramobile ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS kalgebramobile.desktop DESTINATION ${DESKTOPFILE_INSTALL} )
install(FILES
	kalgebrascript.desktop
	DESTINATION ${SERVICETYPES_INSTALL_DIR}
)
