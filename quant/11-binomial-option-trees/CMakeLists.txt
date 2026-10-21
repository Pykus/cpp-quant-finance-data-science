cmake_minimum_required(VERSION 3.16)
project(binomial_option_trees LANGUAGES CXX)

add_library(binomial INTERFACE)
target_include_directories(binomial INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_features(binomial INTERFACE cxx_std_20)

add_executable(binomial_demo main.cpp)
target_link_libraries(binomial_demo PRIVATE binomial)

add_executable(binomial_tests tests.cpp)
target_link_libraries(binomial_tests PRIVATE binomial)

include(CTest)
if(BUILD_TESTING)
    add_test(NAME binomial_tests COMMAND binomial_tests)
endif()
