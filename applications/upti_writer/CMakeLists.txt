cargo_build(NAME uic_upti_writer_build MANIFEST_PATH ${CMAKE_CURRENT_LIST_DIR}
            DEPENDS unify)
cargo_as_cmake_target(uic_upti_writer_build uic-upti-writer)
