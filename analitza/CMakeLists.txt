project(analitza)

add_subdirectory(tests)

set(analitza_SRCS
	builtinmethods.cpp
	providederivative.cpp
	abstractexpressiontransformer.cpp
	expression.cpp
	expressionwriter.cpp
	stringexpressionwriter.cpp
	mathmlexpressionwriter.cpp
	mathmlpresentationexpressionwriter.cpp
	htmlexpressionwriter.cpp
	analyzer.cpp
	container.cpp
	operator.cpp
	operations.cpp
	variables.cpp
	object.cpp
	value.cpp
	variable.cpp
	vector.cpp
	list.cpp
	apply.cpp
	customobject.cpp
	
	expressiontype.cpp
	expressiontypechecker.cpp
	
	expressiontable.cpp	#generated
	expressionparser.cpp	#   ''
	explexer.cpp
	abstractlexer.cpp
	mathmlpresentationlexer.cpp
	analitzautils.cpp
	substituteexpression.cpp
)

kde4_add_library(analitza SHARED ${analitza_SRCS})
target_link_libraries(analitza ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY} ${KDE4_KDECORE_LIBS})

set_target_properties(analitza PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

install(TARGETS analitza ${INSTALL_TARGETS_DEFAULT_ARGS})

if(INSTALL_ANALITZA_HEADERS)
	install(FILES 
		abstractlexer.h
		analitzaexport.h
		analyzer.h
		analitzautils.h
		expression.h
		object.h
		operations.h
		operator.h
		value.h
		variables.h
		expressiontype.h
		builtinmethods.h
		DESTINATION ${INCLUDE_INSTALL_DIR}/analitza/ COMPONENT Devel)
	
	install(FILES
	#		../FindAnalitza.cmake
			DESTINATION ${DATA_INSTALL_DIR}/cmake/modules)
endif(INSTALL_ANALITZA_HEADERS)
