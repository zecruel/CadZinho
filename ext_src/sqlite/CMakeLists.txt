cmake_minimum_required(VERSION 3.7)

file(GLOB C_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

add_library(sqlite ${C_SOURCES})

target_include_directories(sqlite PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Define macros for build and usage
target_compile_definitions(sqlite
    PRIVATE   SQLITE_THREADSAFE=0        # Used internally in the library
    PRIVATE   SQLITE_OMIT_LOAD_EXTENSION
    #INTERFACE USING_MYLIB           # Used by consumers
)

target_compile_features(sqlite PRIVATE c_std_99)