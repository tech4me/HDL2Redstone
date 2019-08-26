set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-header-filter=.*;-checks=cppcoreguidelines-*,google-*,hicpp-*;-warnings-as-errors=*")

add_subdirectory(lib)

add_executable(${PROJECT_NAME} HDL2Redstone.cpp)
target_clangformat_setup(${PROJECT_NAME})
