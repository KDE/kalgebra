add_executable(docbook_analitzacommands main.cpp)
target_link_libraries(docbook_analitzacommands ${ANALITZAGUI_LIBRARY} Qt5::Widgets Qt5::Core)
