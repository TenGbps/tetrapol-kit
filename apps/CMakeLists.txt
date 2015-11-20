include_directories (../lib)

add_executable (tetrapol_dump tetrapol_dump.c)
target_link_libraries (tetrapol_dump tetrapol)

add_executable (tetrapol_build tetrapol_build.c)
target_link_libraries (tetrapol_build tetrapol)
