
include_directories(${CMAKE_SOURCE_DIR}/kalgebra/src/ ${CMAKE_SOURCE_DIR}/kalgebra/src/analitza)

set( analitzatest_SRCS analitzatest.cpp)
kde4_add_unit_test(analitzatest ${analitzatest_SRCS})
target_link_libraries(analitzatest ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS} analitza)

set( expressiontest_SRCS expressiontest.cpp)
kde4_add_unit_test(expressiontest ${expressiontest_SRCS})
target_link_libraries(expressiontest ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS} analitza)

