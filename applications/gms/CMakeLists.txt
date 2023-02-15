cargo_build(NAME uic_gms_build MANIFEST_PATH ${CMAKE_CURRENT_LIST_DIR} DEPENDS
            unify)
cargo_as_cmake_target(uic_gms_build "uic-gms")
