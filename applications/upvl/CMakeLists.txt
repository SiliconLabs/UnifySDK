cargo_build(NAME uic_upvl_build MANIFEST_PATH ${CMAKE_CURRENT_LIST_DIR} DEPENDS
            unify)
cargo_as_cmake_target(uic_upvl_build "uic-upvl")
