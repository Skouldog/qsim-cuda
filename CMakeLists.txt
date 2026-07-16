cmake_minimum_required(VERSION 3.15)



project(qsim-cuda LANGUAGES CXX)

option(QSIM_USE_CUDA "Build with CUDA backend" OFF)
option(QSIM_COVERAGE "Enable coverage instrumentation" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
#Version 1.17.0
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/52eb8108c5bdec04579160ae17225d66034bd723.zip
)
FetchContent_MakeAvailable(googletest)

add_library(qsim_core src/state.cpp src/gates.cpp)
target_include_directories(qsim_core PUBLIC include)

if(QSIM_USE_CUDA)
    enable_language(CUDA)
    target_compile_definitions(qsim_core PUBLIC QSIM_USE_CUDA)
    # toDO add CU files
endif()

if(QSIM_COVERAGE)
    target_compile_options(qsim_core PUBLIC --coverage -O0 -g)
    target_link_options(qsim_core PUBLIC --coverage)
endif()

add_executable(qsim_cli src/main.cpp)
target_link_libraries(qsim_cli PRIVATE qsim_core)



add_executable(qsim_tests  
tests/gates_test.cpp 
tests/state_test.cpp
tests/test_util.cpp
)
target_link_libraries(qsim_tests PRIVATE qsim_core GTest::gtest_main)

add_executable(qsim_bench benchmarks/main.cpp)
target_link_libraries(qsim_bench PRIVATE qsim_core)



enable_testing()

include(GoogleTest)
gtest_discover_tests(qsim_tests)


