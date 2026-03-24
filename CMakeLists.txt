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

# Shared common library (ats_common)
# Code shared between the client and server: packet structures,
# protocol logic, logging utilities, etc.
set(COMMON_SOURCES
        src/common/packets.cpp
        src/common/logging.cpp
)

set(COMMON_HEADERS
        src/common/packets.h
        src/common/logging.h
)

add_library(ats_common STATIC ${COMMON_SOURCES} ${COMMON_HEADERS})

target_include_directories(ats_common PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(ats_common PUBLIC
        ${PLATFORM_SOCKET_LIBS}
)

# Server executable (ats_server)
set(SERVER_SOURCES
        src/server/main.cpp
)

set(SERVER_HEADERS
        src/server/main.h
)

add_executable(ats_server ${SERVER_SOURCES} ${SERVER_HEADERS})

target_include_directories(ats_server PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/server
)

target_link_libraries(ats_server PRIVATE
        ats_common
)

# Client executable (ats_client)
set(CLIENT_SOURCES
        src/client/main.cpp
)

set(CLIENT_HEADERS
        src/client/main.h
)

add_executable(ats_client ${CLIENT_SOURCES} ${CLIENT_HEADERS})

target_include_directories(ats_client PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/client
)

target_link_libraries(ats_client PRIVATE
        ats_common
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
        tests/test_packets.cpp
        tests/test_logging.cpp
        tests/test_server.cpp
        tests/test_client.cpp
)

add_executable(ats_tests ${TEST_SOURCES})

target_include_directories(ats_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(ats_tests PRIVATE
        GTest::gtest_main
        ats_common
)

include(GoogleTest)
gtest_discover_tests(ats_tests)

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
            ANALYZE ats_common ats_server ats_client
            MODE MISRA:1,2,3
            LOG "${CMAKE_BINARY_DIR}/pvs-report.log"
            FORMAT fullhtml
            OUTPUT "${CMAKE_BINARY_DIR}/pvs-report"
            CFG "${CMAKE_SOURCE_DIR}/pvs-studio.cfg"
    )
endif()
