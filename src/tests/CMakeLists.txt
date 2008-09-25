include_directories(${CMAKE_SOURCE_DIR}/kalgebra/src ${CMAKE_SOURCE_DIR}/kalgebra/analitza)

set( operatorsmodeltest_SRCS operatorsmodeltest.cpp ../operatorsmodel.cpp)
kde4_add_unit_test(operatorsmodeltest ${operatorsmodeltest_SRCS})
target_link_libraries(operatorsmodeltest ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY} analitza)

set( functiontest_SRCS functiontest.cpp ../function.cpp ../functionimpl.cpp)
kde4_add_unit_test(functiontest ${functiontest_SRCS})
target_link_libraries(functiontest ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY} analitza)

