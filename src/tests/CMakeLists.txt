# if(KDE4_BUILD_TESTS)

include_directories(${CMAKE_SOURCE_DIR}/kalgebra/src/)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

set( kalgebraCore_SRCS ../analitza.cpp ../variables.cpp ../expression.cpp ../container.cpp ../object.cpp ../operator.cpp ../value.cpp ../exp.cpp)

set( analitzatest_SRCS analitzatest.cpp ${kalgebraCore_SRCS})
kde4_automoc(${analitzatest_SRCS})
kde4_add_executable(analitzatest NOGUI ${analitzatest_SRCS})
target_link_libraries(analitzatest ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS})

set( expressiontest_SRCS expressiontest.cpp ${kalgebraCore_SRCS})
kde4_automoc(${expressiontest_SRCS})
kde4_add_executable(expressiontest NOGUI ${expressiontest_SRCS})
target_link_libraries(expressiontest ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS})

add_test(analitzatest ${EXECUTABLE_OUTPUT_PATH})
add_test(expressiontest ${EXECUTABLE_OUTPUT_PATH})
# endif(KDE4_BUILD_TESTS)
