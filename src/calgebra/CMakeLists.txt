include_directories(${CMAKE_SOURCE_DIR}/kalgebra/src/analitza)

set(calgebra_SRCS
	main.cpp
)

kde4_add_executable(calgebra ${calgebra_SRCS})
target_link_libraries(calgebra ${QT_QTCORE_LIBRARY} ${READLINE_LIBRARY} analitza)

install(TARGETS calgebra DESTINATION ${BIN_INSTALL_DIR})

