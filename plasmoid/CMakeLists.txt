project(kalgebra-plasmoid)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${CMAKE_SOURCE_DIR}/kalgebra/analitza)
set(kalgebraplasma_SRCS kalgebraplasma.cpp)

kde4_add_plugin(kalgebra_plasmoid ${kalgebraplasma_SRCS})
target_link_libraries(kalgebra_plasmoid ${PLASMA_LIBS} ${KDE4_KDEUI_LIBS} analitza)

install(TARGETS kalgebra_plasmoid DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kalgebraplasmoid.desktop DESTINATION ${SERVICES_INSTALL_DIR})
