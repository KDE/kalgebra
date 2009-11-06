set(testLibs ${QT_QTXML_LIBRARY} ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDE4_KDECORE_LIBS} analitza analitzagui)

set( operatorsmodeltest_SRCS operatorsmodeltest.cpp)
kde4_add_unit_test(operatorsmodeltest ${operatorsmodeltest_SRCS})
target_link_libraries(operatorsmodeltest ${testLibs})
