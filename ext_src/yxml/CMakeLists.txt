cmake_minimum_required(VERSION 3.7)

file(GLOB C_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

add_library(yxml ${C_SOURCES})

target_include_directories(yxml PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(yxml PRIVATE c_std_99)