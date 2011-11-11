add_subdirectory(plugins)

if(${MOBILE_BACKEND} STREQUAL "kde")
	add_definitions(-DKDECOMPONENTS)
endif(${MOBILE_BACKEND} STREQUAL "kde")

kde4_add_executable(kalgebramobile pluginsmodel.cpp graph2dmobile.cpp analitzawrapper.cpp
	kalgebramobile.cpp main.cpp)

target_link_libraries(kalgebramobile ${KDE4_KDECORE_LIBS} ${QT_QTDECLARATIVE_LIBRARY}
	${QT_QTGUI_LIBRARY} ${ANALITZA_LIBRARY} ${ANALITZAGUI_LIBRARY})

set(DESKTOPFILE_INSTALL ${XDG_APPS_INSTALL_DIR})
if(${MOBILE_BACKEND} STREQUAL "harmattan")
	set(DESKTOPFILE_INSTALL ${XDG_APPS_INSTALL_DIR}/..)
elseif(${MOBILE_BACKEND} STREQUAL "kde")
	target_link_libraries(kalgebramobile kdeclarative)
endif(${MOBILE_BACKEND} STREQUAL "harmattan")

install(TARGETS kalgebramobile ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS kalgebramobile.desktop DESTINATION ${DESKTOPFILE_INSTALL} )
install(FILES
	kalgebrascript.desktop
	DESTINATION ${SERVICETYPES_INSTALL_DIR}
)
