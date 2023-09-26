include(CheckFunctionExists)
include(CMakePushCheckState)

include_directories(${READLINE_INCLUDE_DIR} ${CURSES_INCLUDE_DIR})

# check if we have recent version of Readline
cmake_push_check_state(RESET)
set(CMAKE_REQUIRED_LIBRARIES ${READLINE_LIBRARY} ${CURSES_LIBRARIES})
check_function_exists(free_history_entry HAVE_FREE_HISTORY_ENTRY)
cmake_pop_check_state()
if(HAVE_FREE_HISTORY_ENTRY)
    add_definitions(-DHAVE_FREE_HISTORY_ENTRY)
endif()

add_executable(calgebra main.cpp)
target_link_libraries(calgebra Qt::Core ${READLINE_LIBRARY} KF6::Analitza)

install(TARGETS calgebra ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

