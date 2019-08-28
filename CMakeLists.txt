cmake_minimum_required(VERSION 3.10)
project(HDL2Redstone)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED)

# Adding cmake dir to module path
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

add_subdirectory(HDL2Redstone)
add_subdirectory(third_party)

set(CLANG_FORMAT_EXCLUDE_PATTERNS "third_party/" "build/" ${CMAKE_BINARY_DIR})
find_package(ClangFormat)
