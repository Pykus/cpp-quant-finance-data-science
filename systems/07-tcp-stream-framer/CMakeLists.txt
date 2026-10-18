cmake_minimum_required(VERSION 3.16)
project(bounded_tcp_stream_framer LANGUAGES CXX)

add_library(framing INTERFACE)
target_include_directories(framing INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(framing INTERFACE cxx_std_20)

add_executable(stream_demo main.cpp)
target_link_libraries(stream_demo PRIVATE framing)

add_executable(stream_tests tests.cpp)
target_link_libraries(stream_tests PRIVATE framing)

if(MSVC)
    target_compile_options(stream_demo PRIVATE /W4 /WX)
    target_compile_options(stream_tests PRIVATE /W4 /WX)
else()
    target_compile_options(stream_demo PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(stream_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

enable_testing()
add_test(NAME stream_decoder_checks COMMAND stream_tests)
