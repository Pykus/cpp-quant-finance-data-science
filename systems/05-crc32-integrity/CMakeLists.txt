cmake_minimum_required(VERSION 3.16)
project(crc32_packet_integrity LANGUAGES CXX)

add_executable(crc32_demo main.cpp)
add_executable(crc32_tests tests.cpp)

foreach(target crc32_demo crc32_tests)
    target_compile_features(${target} PRIVATE cxx_std_20)
    set_target_properties(${target} PROPERTIES
        CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /WX)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
endforeach()

enable_testing()
add_test(NAME crc32_integrity COMMAND crc32_tests)
