cmake_minimum_required(VERSION 3.10)
project(SystemMonitor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Curses REQUIRED)

include_directories(include)
include_directories(${CURSES_INCLUDE_DIR})

add_executable(system_monitor
    src/main.cpp
    src/system_monitor.cpp
    src/process_monitor.cpp
    src/display.cpp
    src/config.cpp
)

target_link_libraries(system_monitor 
    ${CURSES_LIBRARIES}
    stdc++fs
)

if(MSVC)
    target_compile_options(system_monitor PRIVATE /W4)
else()
    target_compile_options(system_monitor PRIVATE -Wall -Wextra -Wpedantic)
endif()

install(TARGETS system_monitor DESTINATION bin)

configure_file(${CMAKE_SOURCE_DIR}/system_monitor.conf ${CMAKE_BINARY_DIR}/system_monitor.conf COPYONLY)

install(FILES ${CMAKE_SOURCE_DIR}/system_monitor.conf DESTINATION etc)
