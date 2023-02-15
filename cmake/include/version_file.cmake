# This is for generating cmake/release-version.cmake to add to source package
configure_file(${CMAKE_CURRENT_LIST_DIR}/../release-version.cmake.in
               ${CMAKE_BINARY_DIR}/cmake/release-version.cmake)
# This is for generating include/uic_version.h to add to source package
configure_file(${CMAKE_CURRENT_LIST_DIR}/../../include/uic_version.h.in
               ${CMAKE_BINARY_DIR}/include/uic_version.h)

if (NOT TARGET uic_source)
    add_library(unify_version INTERFACE IMPORTED GLOBAL)
    target_include_directories(unify_version INTERFACE ${CMAKE_BINARY_DIR}/include)
    add_dependencies(unify_version ${CMAKE_BINARY_DIR}/include/uic_version.h)

    add_custom_target(
    uic_source
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMAND cp ${CMAKE_BINARY_DIR}/cmake/release-version.cmake
            ${CMAKE_CURRENT_LIST_DIR}/../release-version.cmake
    COMMAND ${CMAKE_MAKE_PROGRAM} package_source
    COMMAND rm ${CMAKE_CURRENT_LIST_DIR}/../release-version.cmake)
endif()
