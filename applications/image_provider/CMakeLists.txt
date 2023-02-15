cargo_build(NAME uic_image_provider_build MANIFEST_PATH
            ${CMAKE_CURRENT_LIST_DIR} DEPENDS unify)
cargo_as_cmake_target(uic_image_provider_build uic-image-provider)
