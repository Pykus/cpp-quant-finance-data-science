cmake_minimum_required(VERSION 3.16)
project(historical_tail_risk LANGUAGES CXX)

add_library(tail_risk INTERFACE)
target_include_directories(tail_risk INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(tail_risk INTERFACE cxx_std_20)

add_executable(tail_risk_demo main.cpp)
target_link_libraries(tail_risk_demo PRIVATE tail_risk)

add_executable(tail_risk_tests tests.cpp)
target_link_libraries(tail_risk_tests PRIVATE tail_risk)

if(MSVC)
    target_compile_options(tail_risk_demo PRIVATE /W4)
    target_compile_options(tail_risk_tests PRIVATE /W4)
else()
    target_compile_options(tail_risk_demo PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(tail_risk_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()
add_test(NAME historical_tail_risk_checks COMMAND tail_risk_tests)