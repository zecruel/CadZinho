cmake_minimum_required(VERSION 3.0...3.28)

file(GLOB CPP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(manifold ${CPP_SOURCES})

target_include_directories(manifold PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Define macros for build and usage
target_compile_definitions(manifold
    PRIVATE   MANIFOLD_PAR=-1        # Used internally in the library
    PRIVATE   MANIFOLD_CROSS_SECTION
    #INTERFACE USING_MYLIB           # Used by consumers
)

target_compile_features(manifold PRIVATE cxx_std_17)

target_link_libraries(manifold PUBLIC clipper2)
