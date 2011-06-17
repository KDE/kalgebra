add_subdirectory(scripts)

kde4_add_executable(kalgebramobile
	functionsdialog.cpp widgetswrappers.cpp analitzawrapper.cpp kalgebramobile.cpp uiconfig.cpp main.cpp console.cpp)

target_link_libraries(kalgebramobile ${KDE4_KDEUI_LIBS} ${QT_QTSCRIPT_LIBRARY} ${QT_QTDECLARATIVE_LIBRARY}
# 				${QT_QTSCRIPTTOOLS_LIBRARY}
				analitza analitzagui)

install(TARGETS kalgebramobile ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS kalgebramobile.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install(FILES
	kalgebrascript.desktop
	DESTINATION ${SERVICETYPES_INSTALL_DIR}
)
