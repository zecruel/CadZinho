cmake_minimum_required(VERSION 3.0...3.28)

# Create an option to switch between a system sdl library and a vendored sdl library
option(LIBS_VENDORED "Use vendored libraries" OFF)

if(LIBS_VENDORED)
    #include(${PROJECT_SOURCE_DIR}/sdl2/cmake/sdl2-config.cmake)
    #include(${PROJECT_SOURCE_DIR}/sdl2_net/cmake/sdl2_net-config.cmake)
    set(CMAKE_PREFIX_PATH ${PROJECT_SOURCE_DIR}/glew/;${PROJECT_SOURCE_DIR}/sdl2/;${PROJECT_SOURCE_DIR}/sdl2_net/)
endif()

find_package(GLEW REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_net REQUIRED)

file(GLOB C_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

if (WIN32)
  add_executable(cadzinho ${C_SOURCES} ${PROJECT_SOURCE_DIR}/windows/cz.rc)
  target_include_directories(cadzinho PUBLIC ${PROJECT_SOURCE_DIR}/windows)
else ()
  add_executable(cadzinho ${C_SOURCES})
endif()

target_include_directories(cadzinho PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(cadzinho PUBLIC ${PROJECT_SOURCE_DIR}/ext_src/nanosvg) # header only library
target_include_directories(cadzinho PUBLIC ${PROJECT_SOURCE_DIR}/ext_src/nuklear) # header only library
target_include_directories(cadzinho PUBLIC ${PROJECT_SOURCE_DIR}/ext_src/stb) # header only library
target_include_directories(cadzinho PUBLIC ${PROJECT_SOURCE_DIR}/ext_src/strpool) # header only library
target_include_directories(cadzinho PUBLIC ${PROJECT_SOURCE_DIR}/ext_src/kgflags) # header only library

# Define macros for build and usage
#target_compile_definitions(manifold
#    PRIVATE   MANIFOLD_PAR=-1        # Used internally in the library
#    PRIVATE   MANIFOLD_CROSS_SECTION
#    #INTERFACE USING_MYLIB           # Used by consumers
#)

target_compile_features(cadzinho PRIVATE c_std_99)

target_link_libraries(cadzinho PUBLIC clipper2
PUBLIC manifold PUBLIC dirent PUBLIC lua PUBLIC miniz PUBLIC pdfgen PUBLIC yxml PUBLIC sqlite)



# SDL2::SDL2main may or may not be available. It is e.g. required by Windows GUI applications
if(TARGET SDL2::SDL2main)
    # It has an implicit dependency on SDL2 functions, so it MUST be added before SDL2::SDL2 (or SDL2::SDL2-static)
    target_link_libraries(cadzinho PRIVATE SDL2::SDL2main)
endif()

# Link to the actual SDL2 library. SDL2::SDL2 is the shared SDL library, SDL2::SDL2-static is the static SDL libarary.
target_link_libraries(cadzinho PRIVATE SDL2::SDL2)
target_link_libraries(cadzinho PRIVATE SDL2_net::SDL2_net)

if(MINGW)
    target_link_libraries(cadzinho PRIVATE ws2_32)
endif()

target_link_libraries(cadzinho PRIVATE GLEW::GLEW)



#target_link_libraries(cadzinho PUBLIC opengl32)
#target_link_libraries(cadzinho PUBLIC Shell32)
