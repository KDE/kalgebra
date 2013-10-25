project(analitza-declarative)

include_directories(
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}
)

add_library(analitzadeclarativeplugin SHARED analitzadeclarativeplugin.cpp graph2dmobile.cpp analitzawrapper.cpp)

target_link_libraries(analitzadeclarativeplugin
	Qt5::Core Qt5::Gui Qt5::Qml Qt5::Quick
	${ANALITZA_LIBRARY} ${ANALITZAGUI_LIBRARY} ${ANALITZAPLOT_LIBRARY})

install(TARGETS analitzadeclarativeplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/analitza)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/analitza)

install( FILES
        qml/Graph2D.qml
        DESTINATION ${QML_INSTALL_DIR}/org/kde/analitza
)
