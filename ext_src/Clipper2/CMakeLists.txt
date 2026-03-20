cmake_minimum_required(VERSION 3.0...3.28)

file(GLOB CPP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(clipper2 ${CPP_SOURCES})

target_include_directories(clipper2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_features(clipper2 PRIVATE cxx_std_17)
