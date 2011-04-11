set(qtmmlwidget_LIB_SRCS
    qtmmlwidget.cpp
)

kde4_add_library(qtmmlwidget STATIC ${qtmmlwidget_LIB_SRCS})

target_link_libraries(qtmmlwidget  ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTXML_LIBRARY})

#install(TARGETS qtmmlwidget ${INSTALL_TARGETS_DEFAULT_ARGS})
