cmake_minimum_required(VERSION 3.10)
project(HDL2Redstone)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/third_party/clangformat-cmake/cmake)
include(ClangFormat)

add_subdirectory(HDL2Redstone)
add_subdirectory(third_party)
