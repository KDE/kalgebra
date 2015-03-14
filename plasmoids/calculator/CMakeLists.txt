project(kalgebra-plasmoid)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
set(kalgebraplasma_SRCS kalgebraplasma.cpp)

add_library(plasma_applet_kalgebra MODULE ${kalgebraplasma_SRCS})
target_link_libraries(plasma_applet_kalgebra ${KF5::plasma}  KF5::Analitza)

set(plugin_dir ${KDE_INSTALL_PLUGINDIR}/plugins/kalgebra)
install(TARGETS plasma_applet_kalgebra DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES kalgebraplasmoid.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
