set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(${CMAKE_SOURCE_DIR}/kalgebra/src/ ${CMAKE_SOURCE_DIR}/kalgebra/src/analitza)

set( analitzatest_SRCS analitzatest.cpp)
kde4_add_unit_test(analitzatest ${analitzatest_SRCS})
target_link_libraries(analitzatest ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} analitza)

set( expressiontest_SRCS expressiontest.cpp)
kde4_add_unit_test(expressiontest ${expressiontest_SRCS})
target_link_libraries(expressiontest ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} analitza)

set( functiontest_SRCS functiontest.cpp ../function.cpp ../functionimpl.cpp)
kde4_add_unit_test(functiontest ${functiontest_SRCS})
target_link_libraries(functiontest ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY} analitza)

set( exptest_SRCS exptest.cpp)
kde4_add_unit_test(exptest ${exptest_SRCS})
target_link_libraries(exptest ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} analitza)