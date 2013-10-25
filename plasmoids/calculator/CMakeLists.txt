project(kalgebra-plasmoid)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
set(kalgebraplasma_SRCS kalgebraplasma.cpp)

add_library(plasma_applet_kalgebra MODULE ${kalgebraplasma_SRCS})
target_link_libraries(plasma_applet_kalgebra ${KF5::plasma}  ${ANALITZA_LIBRARY})

set(plugin_dir ${PLUGIN_INSTALL_DIR}/plugins/kalgebra)
install(TARGETS plasma_applet_kalgebra DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kalgebraplasmoid.desktop DESTINATION ${SERVICES_INSTALL_DIR})
