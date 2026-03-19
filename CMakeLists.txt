cmake_minimum_required(VERSION 3.7)

if (WIN32)
    message(STATUS "Building on Windows")
elseif (APPLE)
    message(STATUS "Building on macOS")
elseif (UNIX)
    message(STATUS "Building on Linux/Unix")
endif()

message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Version: ${CMAKE_CXX_COMPILER_VERSION}")

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message(STATUS "Using Microsoft Visual C++")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "Using GCC")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(STATUS "Using Clang")
endif()




project(CadZinho)

add_subdirectory(ext_src/Clipper2) # The library
add_subdirectory(ext_src/manifold) # The library
add_subdirectory(ext_src/lua) # The library
add_subdirectory(ext_src/miniz) # The library
add_subdirectory(ext_src/pdfgen) # The library
add_subdirectory(ext_src/yxml) # The library
add_subdirectory(ext_src/dirent) # The library
add_subdirectory(ext_src/sqlite) # The library
add_subdirectory(src)     # The executable