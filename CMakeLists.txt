cmake_minimum_required(VERSION 3.10)
project(SystemMonitor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Curses REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PROCPS REQUIRED libprocps)

# direct support for cuda and NVML
include_directories(/opt/cuda/targets/x86_64-linux/include)
find_library(NVML_LIBRARY NAMES nvidia-ml PATHS /opt/cuda/targets/x86_64-linux/lib64 /opt/cuda/lib64 /usr/lib64 /usr/lib/x86_64-linux-gnu)
if(NOT NVML_LIBRARY)
    message(FATAL_ERROR "NVML library not found")
endif()

find_library(CUDA_RUNTIME_LIBRARY NAMES cudart PATHS /opt/cuda/targets/x86_64-linux/lib64 /opt/cuda/lib64)
if(NOT CUDA_RUNTIME_LIBRARY)
    message(FATAL_ERROR "CUDA runtime library not found")
endif()

include_directories(include)
include_directories(${CURSES_INCLUDE_DIR})
include_directories(${PROCPS_INCLUDE_DIRS})

add_executable(system_monitor
    src/main.cpp
    src/system_monitor.cpp
    src/process_monitor.cpp
    src/process_monitor_thread.cpp
    src/display.cpp
    src/config.cpp
    src/logger.cpp
    src/gpu_monitor.cpp
    src/network_monitor.cpp
    src/battery_monitor.cpp
)

target_link_libraries(system_monitor 
    ${CURSES_LIBRARIES}
    stdc++fs
    ${NVML_LIBRARY}
    ${CUDA_RUNTIME_LIBRARY}
    ${PROCPS_LIBRARIES}
    dl
    pthread
)

link_directories(${PROCPS_LIBRARY_DIRS})

