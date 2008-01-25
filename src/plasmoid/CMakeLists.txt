project(kalgebra-plasmoid)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${CMAKE_SOURCE_DIR}/kalgebra/src/analitza)
set(kalgebraplasma_SRCS kalgebraplasma.cpp)

kde4_add_plugin(kalgebraplasmoid ${kalgebraplasma_SRCS})
target_link_libraries(kalgebraplasmoid ${PLASMA_LIBS} ${KDE4_KDEUI_LIBS} analitza)

install(TARGETS kalgebraplasmoid DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kalgebraplasmoid.desktop DESTINATION ${SERVICES_INSTALL_DIR})
