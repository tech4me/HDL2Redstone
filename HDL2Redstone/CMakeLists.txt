set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-header-filter=.*;-checks=cppcoreguidelines-*,google-*,hicpp-*;-warnings-as-errors=*")

add_subdirectory(lib)

add_executable(${PROJECT_NAME} HDL2Redstone.cpp)
target_link_libraries(${PROJECT_NAME} schematic)
target_compile_options(${PROJECT_NAME} PRIVATE -Wpedantic -Wall -Wextra -Werror)
target_clangformat_setup(${PROJECT_NAME})
