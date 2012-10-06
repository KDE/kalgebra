project(analitza-declarative)

include_directories(
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}
)

kde4_add_library(analitzadeclarativeplugin SHARED analitzadeclarativeplugin.cpp graph2dmobile.cpp analitzawrapper.cpp)

target_link_libraries(analitzadeclarativeplugin ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTDECLARATIVE_LIBRARY} ${ANALITZA_LIBRARY} ${ANALITZAGUI_LIBRARY} ${ANALITZAPLOT_LIBRARY})

install(TARGETS analitzadeclarativeplugin DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/analitza)
install(FILES qmldir DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/analitza)