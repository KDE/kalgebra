project(analitzagui)

add_subdirectory(tests)

set(analitzagui_SRCS
	operatorsmodel.cpp
	algebrahighlighter.cpp
	variablesmodel.cpp
	expressionedit.cpp

	function.cpp
	functionimpl.cpp
	functionfactory.cpp
	functionpolar.cpp
	functioncartesian.cpp
	functionparametric.cpp
	functionimplicit.cpp

	functionsmodel.cpp
	functionedit.cpp
	graph2d.cpp
)

kde4_add_library(analitzagui SHARED ${analitzagui_SRCS})
target_link_libraries(analitzagui ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDE4_KDEUI_LIBS} analitza)

set_target_properties(analitzagui PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

install(TARGETS analitzagui ${INSTALL_TARGETS_DEFAULT_ARGS})

if(INSTALL_ANALITZA_HEADERS)
        install(FILES
		algebrahighlighter.h
		analitzaguiexport.h
		operatorsmodel.h

                DESTINATION ${INCLUDE_INSTALL_DIR}/analitzagui/ COMPONENT Devel)
endif(INSTALL_ANALITZA_HEADERS)
