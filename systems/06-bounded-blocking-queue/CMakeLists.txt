cmake_minimum_required(VERSION 3.16)
project(bounded_blocking_queue LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)
add_executable(queue_demo main.cpp)
add_executable(queue_tests tests.cpp)
target_link_libraries(queue_demo PRIVATE Threads::Threads)
target_link_libraries(queue_tests PRIVATE Threads::Threads)
target_compile_options(queue_demo PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(queue_tests PRIVATE -Wall -Wextra -Wpedantic)

enable_testing()
add_test(NAME bounded_queue_concurrency COMMAND queue_tests)
