cmake_minimum_required (VERSION 2.6)
project (TETRAPOL_kit)

include (CheckCCompilerFlag)
enable_testing ()

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wall")

CHECK_C_COMPILER_FLAG ("-Og" COMPILER_HAC_OG)

if (COMPILER_HAC_OG)
    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Og -g")
else (COMPILER_HAC_OG)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O1 -g")
endif (COMPILER_HAC_OG)

add_subdirectory (apps)
add_subdirectory (lib)

