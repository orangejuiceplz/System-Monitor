cmake_minimum_required(VERSION 3.10)
project(SystemMonitor)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Curses REQUIRED)

# Include directories
include_directories(include)
include_directories(${CURSES_INCLUDE_DIR})

# Add executable
add_executable(system_monitor
    src/main.cpp
    src/system_monitor.cpp
    src/process_monitor.cpp
    src/display.cpp
)

# Link libraries
target_link_libraries(system_monitor 
    ${CURSES_LIBRARIES}
    stdc++fs
)

# Add compiler warnings
if(MSVC)
    target_compile_options(system_monitor PRIVATE /W4)
else()
    target_compile_options(system_monitor PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Install
install(TARGETS system_monitor DESTINATION bin)
