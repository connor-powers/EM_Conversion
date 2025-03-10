cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
include_directories(include)
project(EM_conversion)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)
FetchContent_MakeAvailable(googletest)
add_executable(run src/conversion.cpp src/helper_functions.cpp)
add_executable(unit_test tests/unit_test.cpp)
target_link_libraries(unit_test gtest_main)