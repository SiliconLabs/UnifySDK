cargo_build(NAME uic_nal_build MANIFEST_PATH ${CMAKE_CURRENT_LIST_DIR} DEPENDS
            unify)
cargo_as_cmake_target(uic_nal_build "uic-nal")
