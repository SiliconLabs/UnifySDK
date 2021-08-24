cargo_build(NAME uic-image-provider EXT_LIB uic_log uic_config)
add_dependencies(doxygen uic-image-provider_doc_target)
add_dependencies(doxygen_zip uic-image-provider_doc_zip_target)
