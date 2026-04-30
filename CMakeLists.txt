cmake_minimum_required(VERSION 3.26)
project(FNFCPP VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE_LOWER)
set(EXPORT_PATH "${CMAKE_SOURCE_DIR}/export/${BUILD_TYPE_LOWER}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${EXPORT_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${EXPORT_PATH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${EXPORT_PATH})

set(ZSTD_BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_SHARED   OFF CACHE BOOL "" FORCE)

include(FetchContent)
FetchContent_Declare(
    DirectX-Headers
    GIT_REPOSITORY https://github.com/microsoft/DirectX-Headers.git
    GIT_TAG main
)
FetchContent_MakeAvailable(DirectX-Headers)

add_subdirectory(funkin/third_party/zstd/build/cmake)
add_subdirectory(funkin)