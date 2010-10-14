add_subdirectory(scripts)

kde4_add_executable(kalgebramobile
	functionsdialog.cpp widgetswrappers.cpp analitzawrapper.cpp kalgebramobile.cpp uiconfig.cpp main.cpp console.cpp)

target_link_libraries(kalgebramobile ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTSCRIPT_LIBRARY} ${QT_QTSCRIPTTOOLS_LIBRARY}
				${KDE4_KDEUI_LIBS} analitza analitzagui)

install(TARGETS kalgebramobile ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES kalgebramobile.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install(FILES
	kalgebrascript.desktop
	DESTINATION ${SERVICETYPES_INSTALL_DIR}
)