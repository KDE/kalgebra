project(kalgebra-plasmoid)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
set(kalgebraplasma_SRCS kalgebraplasma.cpp)

kde4_add_plugin(plasma_applet_kalgebra ${kalgebraplasma_SRCS})
target_link_libraries(plasma_applet_kalgebra ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} analitza)

set(plugin_dir ${PLUGIN_INSTALL_DIR}/plugins/kalgebra)
install(TARGETS plasma_applet_kalgebra DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kalgebraplasmoid.desktop DESTINATION ${SERVICES_INSTALL_DIR})
