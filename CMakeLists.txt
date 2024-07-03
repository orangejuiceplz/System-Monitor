cmake_minimum_required(VERSION 3.10)
project(SystemMonitor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Curses REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PROCPS REQUIRED libprocps)

# CUDA and NVML
include_directories(/opt/cuda/targets/x86_64-linux/include)
find_library(NVML_LIBRARY NAMES nvidia-ml PATHS /opt/cuda/targets/x86_64-linux/lib64 /opt/cuda/lib64 /usr/lib64 /usr/lib/x86_64-linux-gnu)
if(NOT NVML_LIBRARY)
    message(FATAL_ERROR "NVML library not found")
endif()

find_library(CUDA_RUNTIME_LIBRARY NAMES cudart PATHS /opt/cuda/targets/x86_64-linux/lib64 /opt/cuda/lib64)
if(NOT CUDA_RUNTIME_LIBRARY)
    message(FATAL_ERROR "CUDA runtime library not found")
endif()

# Include directories
include_directories(include)
include_directories(${CURSES_INCLUDE_DIR})
include_directories(${PROCPS_INCLUDE_DIRS})

# Add executable
add_executable(system_monitor
    src/main.cpp
    src/system_monitor.cpp
    src/process_monitor.cpp
    src/display.cpp
    src/config.cpp
    src/logger.cpp
    src/gpu_monitor.cpp
)

# Link libraries
target_link_libraries(system_monitor 
    ${CURSES_LIBRARIES}
    stdc++fs
    ${NVML_LIBRARY}
    ${CUDA_RUNTIME_LIBRARY}
    ${PROCPS_LIBRARIES}
    dl
)

# Add link directories
link_directories(${PROCPS_LIBRARY_DIRS})

# Add compiler warnings
if(MSVC)
    target_compile_options(system_monitor PRIVATE /W4)
else()
    target_compile_options(system_monitor PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Install
install(TARGETS system_monitor DESTINATION bin)

# Copy configuration file to build directory
configure_file(${CMAKE_SOURCE_DIR}/system_monitor.conf ${CMAKE_BINARY_DIR}/system_monitor.conf COPYONLY)

# Install configuration file
install(FILES ${CMAKE_SOURCE_DIR}/system_monitor.conf DESTINATION etc)
