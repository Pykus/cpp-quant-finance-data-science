cmake_minimum_required(VERSION 3.16)
project(quant_implied_volatility LANGUAGES CXX)

add_library(implied_volatility INTERFACE)
target_include_directories(implied_volatility INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(implied_volatility INTERFACE cxx_std_20)

add_executable(implied_volatility_demo main.cpp)
target_link_libraries(implied_volatility_demo PRIVATE implied_volatility)

add_executable(implied_volatility_tests tests.cpp)
target_link_libraries(implied_volatility_tests PRIVATE implied_volatility)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    foreach(target implied_volatility_demo implied_volatility_tests)
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
    endforeach()
endif()

enable_testing()
add_test(NAME quant_implied_volatility COMMAND implied_volatility_tests)
