cmake_minimum_required(VERSION 4.1)
project(AircraftTrackingSystem)

# Sets minimum C++ version
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Provides compiler warnings for MISRA compliance
if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Platform-Agnostic Socket Configurations
# Uses WS2_32 on Windows, and POSIX otherwise
if(WIN32)
    set(PLATFORM_SOCKET_LIBS ws2_32)
else()
    set(PLATFORM_SOCKET_LIBS "")
endif()

# Source Files which will be compiled
set(SOURCES
        src/main.cpp
)

# Source Headers for discovery/linkage
set(HEADERS
        src/main.h
)

# Build the main executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        ${PLATFORM_SOCKET_LIBS}
)

# Googletest Framework
# Automatically downloads the latest version of Googletest
include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
)

# Prevent GoogleTest from overriding the parent folder's
# compiler/linker settings on Windows.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

enable_testing()

# Add test source files to this list as you create them.
set(TEST_SOURCES
        tests/test_main.cpp
)

add_executable(${PROJECT_NAME}_tests ${TEST_SOURCES})

target_include_directories(${PROJECT_NAME}_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME}_tests PRIVATE
        GTest::gtest_main
        ${PLATFORM_SOCKET_LIBS}
)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}_tests)

# PVS-Studio MISRA Compliance Checker
# Provides a build target for running MISRA compliance checks.
# Requires PVS-Studio to be installed and licensed.
#
# Usage:
#   cmake -B build -G Ninja -DENABLE_PVS_STUDIO=ON
#   cmake --build build --target pvs_analysis
#
# To disable (default), simply omit the flag or set it to OFF.
option(ENABLE_PVS_STUDIO "Enable PVS-Studio MISRA analysis" OFF)

if(ENABLE_PVS_STUDIO)
    include(cmake/PVS-Studio.cmake)

    pvs_studio_add_target(
            TARGET pvs_analysis
            ALL
            ANALYZE ${PROJECT_NAME}
            MODE MISRA:1,2,3
            LOG "${CMAKE_BINARY_DIR}/pvs-report.log"
            FORMAT fullhtml
            OUTPUT "${CMAKE_BINARY_DIR}/pvs-report"
            CFG "${CMAKE_SOURCE_DIR}/pvs-studio.cfg"
    )
endif()
