cmake_minimum_required(VERSION 3.20)
project(antithetic_monte_carlo LANGUAGES CXX)

include(CTest)

add_library(antithetic_pricing INTERFACE)
target_compile_features(antithetic_pricing INTERFACE cxx_std_20)
target_include_directories(antithetic_pricing INTERFACE .)

add_executable(antithetic_demo demo.cpp)
target_link_libraries(antithetic_demo PRIVATE antithetic_pricing)
add_executable(antithetic_tests tests.cpp)
target_link_libraries(antithetic_tests PRIVATE antithetic_pricing)

if(MSVC)
    target_compile_options(antithetic_demo PRIVATE /W4 /WX)
    target_compile_options(antithetic_tests PRIVATE /W4 /WX)
else()
    target_compile_options(antithetic_demo PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(antithetic_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

add_test(NAME antithetic_tests COMMAND antithetic_tests)
