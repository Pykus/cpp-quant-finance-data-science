cmake_minimum_required(VERSION 3.16)
project(black_scholes_greeks LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(greeks_demo main.cpp)
add_executable(greeks_tests tests.cpp)

if(MSVC)
    target_compile_options(greeks_demo PRIVATE /W4)
    target_compile_options(greeks_tests PRIVATE /W4)
else()
    target_compile_options(greeks_demo PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(greeks_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()
add_test(NAME greeks_regression_tests COMMAND greeks_tests)
