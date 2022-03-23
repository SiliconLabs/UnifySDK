cmake_minimum_required(VERSION 3.19)

# ##############################################################################
# Modules
# ##############################################################################
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
                      ${CMAKE_MODULE_PATH})
set(CMAKE_INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/include/"
                       ${CMAKE_INCLUDE_PATH})

# ##############################################################################
# Include Configurations
# ##############################################################################
include(cmake/include/version.cmake)
include(cmake/include/build_permutations.cmake)
include(cmake/include/link_wrap.cmake)

project(
  uic
  LANGUAGES C CXX
  VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REV})

include(cmake/include/target_interface_libraries.cmake)
include(cmake/include/compiler_options.cmake)
include(cmake/include/compatible_platform.cmake)
include(cmake/include/unittest.cmake)
include(cmake/include/doxygen.cmake)
include(cmake/include/package.cmake)
include(cmake/include/uic_helper.cmake)

enable_language(Rust)
set(RUST_MIN_VERSION 1.58.1)
if(CMAKE_Rust_COMPILER_VERSION VERSION_LESS RUST_MIN_VERSION)
  message(
    FATAL_ERROR
      "Rust version is ${CMAKE_Rust_COMPILER_VERSION}, expect at least ${RUST_MIN_VERSION},
      please update using 'rustup update' or similar,
      see https://www.rust-lang.org/tools/install documentation for further details."
  )
endif()
include(CMakeCargo)
# ##############################################################################
# Global includes
# ##############################################################################
include_directories(include ${CMAKE_CURRENT_BINARY_DIR}/include)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/include/sl_status_strings.h
  COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/scripts/sl_status_strings.sh >
          ${CMAKE_CURRENT_BINARY_DIR}/include/sl_status_strings.h
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/sl_status.h
          ${CMAKE_CURRENT_SOURCE_DIR}/scripts/sl_status_strings.sh)
add_custom_target(
  sl_status_strings
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/include/sl_status_strings.h)

# ##############################################################################
# Subdirectories
# ##############################################################################
add_subdirectory(components)
add_subdirectory(applications)

# This is for generating cmake/release-version.cmake to add to source package
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/release-version.cmake.in
               ${CMAKE_BINARY_DIR}/cmake/release-version.cmake)
add_custom_target(
  uic_source
  COMMAND cp ${CMAKE_BINARY_DIR}/cmake/release-version.cmake
          ${CMAKE_CURRENT_SOURCE_DIR}/cmake/release-version.cmake
  COMMAND ${CMAKE_MAKE_PROGRAM} package_source
  COMMAND rm ${CMAKE_CURRENT_SOURCE_DIR}/cmake/release-version.cmake)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/uic_version.h.in
               ${CMAKE_BINARY_DIR}/include/uic_version.h)
