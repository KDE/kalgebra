add_executable(docbook_analitzacommands main.cpp)
target_link_libraries(docbook_analitzacommands ${ANALITZAGUI_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY})
