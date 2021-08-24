SET(lib_source_files
s2_extern_mock.c
s2_mock.c
)

add_library(s2_mock ${lib_source_files})
target_link_libraries(s2_mock mock)

