
include_directories(${CMAKE_SOURCE_DIR}/kalgebra/src/)

set( kalgebraCore_SRCS ../analitza.cpp ../variables.cpp ../expression.cpp ../container.cpp ../object.cpp ../operator.cpp ../value.cpp ../exp.cpp)

set( analitzatest_SRCS analitzatest.cpp ${kalgebraCore_SRCS})
kde4_add_unit_test(analitzatest ${analitzatest_SRCS})
target_link_libraries(analitzatest ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS})

set( expressiontest_SRCS expressiontest.cpp ${kalgebraCore_SRCS})
kde4_add_unit_test(expressiontest ${expressiontest_SRCS})
target_link_libraries(expressiontest ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS})

