include_directories(${CMAKE_SOURCE_DIR}/kalgebra/src ${CMAKE_SOURCE_DIR}/kalgebra/analitza)

set(testLibs ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDE4_KDECORE_LIBS} analitza)

set( operatorsmodeltest_SRCS operatorsmodeltest.cpp ../operatorsmodel.cpp)
kde4_add_unit_test(operatorsmodeltest ${operatorsmodeltest_SRCS})
target_link_libraries(operatorsmodeltest ${testLibs})

set( functiontest_SRCS functiontest.cpp ../function.cpp ../functionimpl.cpp ../functionfactory.cpp
	../functionpolar.cpp ../functioncartesian.cpp ../functionparametric.cpp)
kde4_add_unit_test(functiontest ${functiontest_SRCS})
target_link_libraries(functiontest ${testLibs})

