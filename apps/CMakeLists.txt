include_directories (../lib)

find_package(PkgConfig)
pkg_check_modules(JSON_C REQUIRED json-c)

add_executable (tetrapol_dump tetrapol_dump.c)
target_link_libraries (tetrapol_dump tetrapol)

add_executable (tetrapol_build tetrapol_build.c)
target_link_libraries (tetrapol_build tetrapol ${JSON_C_LIBRARIES} )
