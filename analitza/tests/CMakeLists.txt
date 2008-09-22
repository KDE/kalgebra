set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(${CMAKE_SOURCE_DIR}/kalgebra/analitza)

set( analitzatest_SRCS analitzatest.cpp)
kde4_add_unit_test(analitzatest ${analitzatest_SRCS})
target_link_libraries(analitzatest ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} analitza)

set( expressiontest_SRCS expressiontest.cpp)
kde4_add_unit_test(expressiontest ${expressiontest_SRCS})
target_link_libraries(expressiontest ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} analitza)

set( exptest_SRCS exptest.cpp)
kde4_add_unit_test(exptest ${exptest_SRCS})
target_link_libraries(exptest ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} analitza)

set( mathmlpresentationlexertest_SRCS mathmlpresentationlexertest.cpp)
kde4_add_unit_test(mathmlpresentationlexertest ${mathmlpresentationlexertest_SRCS})
target_link_libraries(mathmlpresentationlexertest ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} analitza)
