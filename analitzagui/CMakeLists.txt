project(analitzagui)

include_directories(..)

add_subdirectory(tests)

set(analitzagui_SRCS
	operatorsmodel.cpp
	algebrahighlighter.cpp
	variablesmodel.cpp

	function.cpp
	functionimpl.cpp
	functionfactory.cpp
	functionpolar.cpp
	functioncartesian.cpp
	functionparametric.cpp
	functionimplicit.cpp

)

kde4_add_library(analitzagui SHARED ${analitzagui_SRCS})
target_link_libraries(analitzagui ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDE4_KDECORE_LIBS} analitza)

set_target_properties(analitzagui PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

install(TARGETS analitzagui ${INSTALL_TARGETS_DEFAULT_ARGS})

if(INSTALL_ANALITZA_HEADERS)
        install(FILES
		algebrahighlighter.h
		analitzaguiexport.h
		operatorsmodel.h

                DESTINATION ${INCLUDE_INSTALL_DIR}/analitzagui/ COMPONENT Devel)
endif(INSTALL_ANALITZA_HEADERS)
