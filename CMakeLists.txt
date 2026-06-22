cmake_minimum_required(VERSION 3.15)


project(qsim-cuda LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_library(qsim_core src/state.cpp)
target_include_directories(qsim_core PUBLIC include)

add_executable(qsim_cli src/main.cpp)
target_link_libraries(qsim_cli PRIVATE qsim_core)

add_executable(qsim_tests tests/main.cpp)
target_link_libraries(qsim_tests PRIVATE qsim_core)

add_executable(qsim_bench benchmarks/main.cpp)
target_link_libraries(qsim_bench PRIVATE qsim_core)


