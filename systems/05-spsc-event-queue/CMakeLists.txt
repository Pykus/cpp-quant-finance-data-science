cmake_minimum_required(VERSION 3.16)
project(spsc_event_queue LANGUAGES CXX)

find_package(Threads REQUIRED)

add_executable(spsc_demo main.cpp)
add_executable(spsc_tests tests.cpp)

foreach(target IN ITEMS spsc_demo spsc_tests)
    target_compile_features(${target} PRIVATE cxx_std_20)
    target_link_libraries(${target} PRIVATE Threads::Threads)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
endforeach()

enable_testing()
add_test(NAME spsc_event_queue_tests COMMAND spsc_tests)
