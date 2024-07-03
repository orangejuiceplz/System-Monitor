cmake_minimum_required(VERSION 3.10)
project(SystemMonitor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(system_monitor
    src/main.cpp
    src/system_monitor.cpp
)

target_link_libraries(system_monitor stdc++fs)
