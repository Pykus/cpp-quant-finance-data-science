cmake_minimum_required(VERSION 3.16)
project(bootstrap_intervals LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(bootstrap_demo main.cpp)
add_executable(bootstrap_tests tests.cpp)
target_compile_options(bootstrap_demo PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(bootstrap_tests PRIVATE -Wall -Wextra -Wpedantic)

enable_testing()
add_test(NAME bootstrap_statistics COMMAND bootstrap_tests)
