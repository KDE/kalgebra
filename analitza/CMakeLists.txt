project(analitza)

add_subdirectory(tests)

set(analitza_SRCS
	expression.cpp
	expressionwriter.cpp
	stringexpressionwriter.cpp
	mathmlexpressionwriter.cpp
	mathmlpresentationexpressionwriter.cpp
	htmlexpressionwriter.cpp
	analitza.cpp
	container.cpp
	operator.cpp
	operations.cpp
	variables.cpp
	object.cpp
	value.cpp
	variable.cpp
	vector.cpp
	list.cpp
	
	expressiontype.cpp
	expressiontypechecker.cpp
	
	expressiontable.cpp	#generated
	expressionparser.cpp	#   ''
	explexer.cpp
	abstractlexer.cpp
	mathmlpresentationlexer.cpp
	analitzautils.cpp
)

kde4_add_library(analitza SHARED ${analitza_SRCS})
target_link_libraries(analitza ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY} ${KDE4_KDECORE_LIBS})

set_target_properties(analitza PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

install(TARGETS analitza ${INSTALL_TARGETS_DEFAULT_ARGS})

if(INSTALL_ANALITZA_HEADERS)
	install(FILES 
		abstractlexer.h
		analitzaexport.h
		analitza.h
		analitzautils.h
		container.h
		explexer.h
		expression.h
		expressionparser.h
		expressiontable_p.h
		expressionwriter.h
		htmlexpressionwriter.h
		list.h
		mathmlexpressionwriter.h
		mathmlpresentationexpressionwriter.h
		mathmlpresentationlexer.h
		object.h
		operations.h
		operator.h
		stringexpressionwriter.h
		value.h
		variable.h
		variables.h
		vector.h
		DESTINATION ${INCLUDE_INSTALL_DIR}/analitza/ COMPONENT Devel)
endif(INSTALL_ANALITZA_HEADERS)

