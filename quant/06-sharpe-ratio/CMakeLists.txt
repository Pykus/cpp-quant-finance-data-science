cmake_minimum_required(VERSION 3.16)
project(sharpe_ratio_lesson LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(sharpe_demo main.cpp)
add_executable(sharpe_tests tests.cpp)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(sharpe_demo PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(sharpe_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

enable_testing()
add_test(NAME sharpe_tests COMMAND sharpe_tests)
