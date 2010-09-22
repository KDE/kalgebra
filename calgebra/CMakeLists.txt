include_directories(${READLINE_INCLUDE_DIR})

set(calgebra_SRCS
	main.cpp
)

kde4_add_executable(calgebra ${calgebra_SRCS})
target_link_libraries(calgebra ${QT_QTCORE_LIBRARY} ${READLINE_LIBRARY} ${NCURSES_LIBRARY} analitza)

install(TARGETS calgebra ${INSTALL_TARGETS_DEFAULT_ARGS})

