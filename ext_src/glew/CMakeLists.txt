cmake_minimum_required(VERSION 3.0...3.28)

file(GLOB C_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

add_library(glew ${C_SOURCES})

target_include_directories(glew PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Define macros for build and usage
target_compile_definitions(glew
    PRIVATE   GLEW_NO_GLU
    PRIVATE   GLEW_STATIC
)

target_compile_features(glew PRIVATE c_std_99)
