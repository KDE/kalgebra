add_subdirectory(declarative)
add_subdirectory(plugins)

if(${MOBILE_BACKEND} STREQUAL "kde")
	add_definitions(-DKDECOMPONENTS)
endif(${MOBILE_BACKEND} STREQUAL "kde")

qt4_add_resources(KALGEBRAMOBILE_SRCS resources.qrc)
kde4_add_executable(kalgebramobile pluginsmodel.cpp
	kalgebramobile.cpp main.cpp ${KALGEBRAMOBILE_SRCS})

target_link_libraries(kalgebramobile ${KDE4_KDECORE_LIBS} ${QT_QTDECLARATIVE_LIBRARY}
	${QT_QTGUI_LIBRARY} ${ANALITZA_LIBRARY} ${ANALITZAGUI_LIBRARY} ${ANALITZAPLOT_LIBRARY})

add_definitions("-DPREFIX=\\\"${CMAKE_INSTALL_PREFIX}\\\"")

set(DESKTOPFILE_INSTALL ${XDG_APPS_INSTALL_DIR})
if(${MOBILE_BACKEND} STREQUAL "harmattan")
	set(DESKTOPFILE_INSTALL ${XDG_APPS_INSTALL_DIR}/..)
elseif(${MOBILE_BACKEND} STREQUAL "kde")
	target_link_libraries(kalgebramobile kdeclarative)
endif(${MOBILE_BACKEND} STREQUAL "harmattan")

set(DESKTOPFILE kalgebramobile.desktop)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/kalgebramobile${MOBILE_BACKEND}.desktop)
	set(DESKTOPFILE kalgebramobile${MOBILE_BACKEND}.desktop)
endif()

install(TARGETS kalgebramobile ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS ${DESKTOPFILE} DESTINATION ${DESKTOPFILE_INSTALL} )
