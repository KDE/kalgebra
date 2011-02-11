set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(${kalgebra_SOURCE_DIR}/analitza)

set(testLibs ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS} analitza)

set( analitzatest_SRCS analitzatest.cpp)
kde4_add_unit_test(analitzatest ${analitzatest_SRCS})
target_link_libraries(analitzatest ${testLibs})

set( expressiontest_SRCS expressiontest.cpp)
kde4_add_unit_test(expressiontest ${expressiontest_SRCS})
target_link_libraries(expressiontest ${testLibs})

set( exptest_SRCS exptest.cpp)
kde4_add_unit_test(exptest ${exptest_SRCS})
target_link_libraries(exptest ${testLibs})

set( mathmlpresentationtest_SRCS mathmlpresentationtest.cpp)
kde4_add_unit_test(mathmlpresentationtest ${mathmlpresentationtest_SRCS})
target_link_libraries(mathmlpresentationtest ${testLibs})

kde4_add_unit_test(typechecktest typechecktest.cpp )
target_link_libraries(typechecktest ${testLibs})
