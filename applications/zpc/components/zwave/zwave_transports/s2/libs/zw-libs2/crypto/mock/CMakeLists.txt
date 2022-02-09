SET(lib_source_files
curve25519_mock.c
kderiv_mock.c
)

add_library(crypto_mocks ${lib_source_files})
target_link_libraries(crypto_mocks mock)
