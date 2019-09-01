set(CMAKE_CXX_CLANG_TIDY "clang-tidy")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(src)

add_executable(${PROJECT_NAME} HDL2Redstone.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE hdl2redstone-core)
target_compile_options(${PROJECT_NAME} PRIVATE -Wpedantic -Wall -Wextra -Werror)
