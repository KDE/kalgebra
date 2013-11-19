add_subdirectory(declarative)
add_subdirectory(plugins)

if(KDeclarative_FOUND)
	add_definitions(-DKDECOMPONENTS)
endif()

qt5_add_resources(KALGEBRAMOBILE_SRCS resources.qrc)
add_executable(kalgebramobile pluginsmodel.cpp
	kalgebramobile.cpp main.cpp ${KALGEBRAMOBILE_SRCS})

target_link_libraries(kalgebramobile Qt5::Qml Qt5::Quick Qt5::Gui
	KF5::Analitza KF5::AnalitzaGui KF5::AnalitzaPlot)

add_definitions("-DPREFIX=\\\"${CMAKE_INSTALL_PREFIX}\\\"")

set(DESKTOPFILE_INSTALL ${XDG_APPS_INSTALL_DIR})
if(${MOBILE_BACKEND} STREQUAL "harmattan")
	set(DESKTOPFILE_INSTALL ${XDG_APPS_INSTALL_DIR}/..)
elseif(KDeclarative_FOUND)
	target_link_libraries(kalgebramobile KF5::KDeclarative)
endif()

set(DESKTOPFILE kalgebramobile.desktop)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/kalgebramobile${MOBILE_BACKEND}.desktop)
	set(DESKTOPFILE kalgebramobile${MOBILE_BACKEND}.desktop)
endif()

# install(TARGETS kalgebramobile ${INSTALL_TARGETS_DEFAULT_ARGS})
# install(PROGRAMS ${DESKTOPFILE} DESTINATION ${DESKTOPFILE_INSTALL} )
