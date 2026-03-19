cmake_minimum_required(VERSION 3.7)

file(GLOB C_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

add_library(dirent ${C_SOURCES})

target_include_directories(dirent PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(dirent PRIVATE c_std_99)