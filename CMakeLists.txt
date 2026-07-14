cmake_minimum_required(VERSION 3.15)


project(qsim-cuda LANGUAGES CXX)

option(QSIM_USE_CUDA "Build with CUDA backend" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_library(qsim_core src/state.cpp src/gates.cpp)
target_include_directories(qsim_core PUBLIC include)

if(QSIM_USE_CUDA)
    enable_language(CUDA)
    target_compile_definitions(qsim_core PUBLIC QSIM_USE_CUDA)
    # toDO add CU files
endif()

add_executable(qsim_cli src/main.cpp)
target_link_libraries(qsim_cli PRIVATE qsim_core)

add_executable(qsim_tests 
tests/main.cpp 
tests/gates_test.cpp 
tests/test_util.cpp 
tests/state_test.cpp
)
target_link_libraries(qsim_tests PRIVATE qsim_core)

add_executable(qsim_bench benchmarks/main.cpp)
target_link_libraries(qsim_bench PRIVATE qsim_core)


