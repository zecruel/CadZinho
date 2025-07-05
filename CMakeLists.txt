cmake_minimum_required(VERSION 3.7)
project(CadZinho LANGUAGES C CXX)

file(GLOB C_SOURCES "src/*.c")
file(GLOB EXT_C_SOURCES "ext_src/*/*.c")
file(GLOB CPP_SOURCES "src/*.cpp")
file(GLOB EXT_CPP_SOURCES "ext_src/*/*.cpp")

add_executable(cadzinho ${C_SOURCES} ${CPP_SOURCES} ${EXT_C_SOURCES} ${EXT_CPP_SOURCES})
target_compile_options(cadzinho PUBLIC
    $<$<COMPILE_LANGUAGE:C>:-DSQLITE_THREADSAFE=0 -DSQLITE_OMIT_LOAD_EXTENSION>  # C-specific flags
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++17 -DMANIFOLD_PAR=-1 -DMANIFOLD_CROSS_SECTION> # C++-specific flags
)

target_include_directories(cadzinho PUBLIC src)

file(GLOB SUBDIRS LIST_DIRECTORIES true "ext_src/*")
foreach(dir ${SUBDIRS})
    IF(IS_DIRECTORY ${dir})
        target_include_directories(cadzinho PUBLIC ${dir})
    ELSE()
        CONTINUE()
    ENDIF()
endforeach()


#set_source_files_properties(my_c_file.c PROPERTIES COMPILE_OPTIONS -O2)