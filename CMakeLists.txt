cmake_minimum_required(VERSION 3.16)
project(qslog)

set(CMAKE_CXX_STANDARD 17)

# clang-tidy
option(QSLOG_CLANG_TIDY_ENABLE "run clang-tidy" ON)
if(QSLOG_CLANG_TIDY_ENABLE AND ${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(CMAKE_CXX_CLANG_TIDY "clang-tidy")
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    message(STATUS "enabled clang-tidy")
endif()

option(QSLOG_BUILD_TESTS "build tests" OFF)
option(QSLOG_BUILD_BENCHMARKS "build benchmarks" ON)
option(QSLOG_BUILD_EXAMPLES "build examples" ON)

include(cmake/module_define.cmake)
message("CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

add_subdirectory(third_party)
add_subdirectory(src)

if(QSLOG_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(QSLOG_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

if(QSLOG_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
