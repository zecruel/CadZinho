cmake_minimum_required(VERSION 3.0...3.28)

file(GLOB C_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

add_library(miniz ${C_SOURCES})

target_include_directories(miniz PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(miniz PRIVATE c_std_99)
