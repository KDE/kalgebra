project(kalgebra-plasmoid)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${CMAKE_SOURCE_DIR}/kalgebra/analitza)
set(kalgebraplasma_SRCS kalgebraplasma.cpp expressionedit.cpp)

kde4_add_plugin(plasma_applet_kalgebra ${kalgebraplasma_SRCS})
target_link_libraries(plasma_applet_kalgebra ${PLASMA_LIBS} ${KDE4_KDEUI_LIBS} analitza)

install(TARGETS plasma_applet_kalgebra DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kalgebraplasmoid.desktop DESTINATION ${SERVICES_INSTALL_DIR})
