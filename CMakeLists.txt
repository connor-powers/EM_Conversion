cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
include_directories(include)
include_directories(/usr/local/eigen-3.4.0)
project(EM_conversion)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)
FetchContent_MakeAvailable(googletest)
add_executable(run src/conversion.cpp src/helper_functions.cpp)
add_executable(test src/unit_test.cpp)
target_link_libraries(test gtest_main)