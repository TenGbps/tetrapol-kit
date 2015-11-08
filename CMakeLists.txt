cmake_minimum_required (VERSION 2.6)
project (TETRAPOL_kit)

include (CheckCCompilerFlag)
enable_testing ()

CHECK_C_COMPILER_FLAG ("-std=c11" COMPILER_HAVE_C11)
if (COMPILER_HAVE_C11)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wall")
else (COMPILER_HAVE_C11)
    CHECK_C_COMPILER_FLAG ("-std=c1x" COMPILER_HAVE_C1X)
    if (COMPILER_HAVE_C1X)
        set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c1x -Wall")
    else (COMPILER_HAVE_C1X)
        set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall")
        message(WARNING "c11/c1x not supported by compiler, falling back to c99.")
    endif (COMPILER_HAVE_C1X)
endif (COMPILER_HAVE_C11)

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_XOPEN_SOURCE")

CHECK_C_COMPILER_FLAG ("-Og" COMPILER_HAS_OG)
if (COMPILER_HAS_OG)
    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Og -g")
else (COMPILER_HAS_OG)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O1 -g")
endif (COMPILER_HAS_OG)

add_subdirectory (apps)
add_subdirectory (lib)

