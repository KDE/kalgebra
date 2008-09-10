project(analitza)

add_subdirectory(tests)

set(analitza_SRCS
	expression.cpp
	expressionwritter.cpp
	stringexpressionwritter.cpp
	mathmlexpressionwritter.cpp
	htmlexpressionwritter.cpp
	analitza.cpp
	container.cpp
	operator.cpp
	operations.cpp
	exp.cpp
	variables.cpp
	object.cpp
	value.cpp
)

kde4_add_library(analitza SHARED ${analitza_SRCS})
target_link_libraries(analitza ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY} ${KDE4_KDECORE_LIBS})

set_target_properties(analitza PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

install(TARGETS analitza ${INSTALL_TARGETS_DEFAULT_ARGS})
