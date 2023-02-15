# cmake-format: off
# This file specifies the build of Rust packages. For usage, skip to function
# `cargo_build`. The goal designing these scripts was to be able to build Rust
# packages as fast as possible. We try to achieve this by having as less as
# custom targets as possible. And let cargo do as much as buildsystem work for
# us. "Dont fight the cargo buildsystem!"
# 1. Cargo is quite aggressive in flagging packages dirty, a slight change in an
# environment variable or configuration will trigger an rebuild. more about
# this in Cargo Fingerprinting.
# 2. Cargo manages a lot of build configuration. based on experience we saw that
# its very hard to inject custom configuration via additional flags without
# it conflicted or even simply being ignored.
#
# In theory you dont have to do anything in order for your new Rust package
# to get build as Cargo will handle this for us. However you still might want
# to declare your Rust packages with `cargo_build` as it enables more
# features:
# - export of your Rust package as actual cmake targets. see `add_cmake_target`
# this means you can integrate it easy into other cmake targets.
# - Debian package generation.
# - building a single Rust package.
#
# These targets will be created on top of the cargo_build.
#
# # Cargo Fingerprinting
# Cargo generates fingerprint files which are used to determin if a rebuild
# should happen. You can view the logging of them by passing the following
# variable to a cargo build:
# `CARGO_LOG=cargo::core::compiler::fingerprint=trace' Part of this fingerprint
# is a metadata hash, you can view it by passing `-vvv` to cargo build and looks
# like `-C metadata=295d6d3a59043349 -C extra-filename=-295d6d3a59043349`. This
# hash is based on various things including Rust compiler version, rustc binary,
# environment, package configuration, passed flags etc. Since we have no means
# to overwrite this metadata hash we are quite limited in what we can do before
# an rebuild happens. One of the reasons why we went for a cmake target to build
# everything. We tried to build specific packages as well, but this again gave
# poor performance.
#
# to see more specifically, whats inculded in the hash, you can take a look in
# the source of cargo metadata:
# src/cargo/core/compiler/context/compilation_files.rs:compute_metadata
# https://github.com/rust-lang/cargo/issues/2904
# https://github.com/rust-lang/cargo/issues/6529
# https://github.com/rust-lang/cargo/pull/8623
#
# cmake-format: on

if(NOT CMAKE_CARGO_INCLUDED)
  set(CMAKE_CARGO_INCLUDED True)
else()
  return()
endif()

include(CargoMeta)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(Python3 REQUIRED)

# This function declares a cmake target for a given rust build target. Name of the
# cmake target: <rust_name>. note: Cmake target names that are equal
# to the filename of output file are not permitted.
function(add_cmake_target CARGO_META CARGO_META_PACKAGE BUILD_TARGET)
  string(JSON CARGO_NAME GET ${CARGO_META_PACKAGE} "name")
  get_target_property(TARGET_DIR ${BUILD_TARGET} CARGO_TARGET_DIR)
  rust_native_target_file(${TARGET_DIR} ${CARGO_META_PACKAGE} OUT_FILE)
  rust_source_path(${CARGO_META_PACKAGE} TARGET_SRC)
  rust_get_build_type(${CARGO_META_PACKAGE} TARGET_TYPE)

  if(${TARGET_TYPE} STREQUAL "bin")
    add_executable(${CARGO_NAME} IMPORTED GLOBAL)
    return()
  elseif(${TARGET_TYPE} STREQUAL "dylib" OR ${TARGET_TYPE} STREQUAL "lib")
    add_library(${CARGO_NAME} SHARED IMPORTED GLOBAL)
  elseif(${TARGET_TYPE} STREQUAL "staticlib")
    add_library(${CARGO_NAME} STATIC IMPORTED GLOBAL)
  endif()

  # jumping through cmake hoops to get the imported cmake project depend on the
  # out file and the outfile depend on the BUILD_TARGET
  add_custom_command(OUTPUT ${OUT_FILE} COMMAND ${CMAKE_COMMAND} -E touch
                                                "${OUT_FILE}")
  add_custom_target(${CARGO_NAME}_out_file ALL DEPENDS "${OUT_FILE}"
                                                       ${BUILD_TARGET})
  #add_dependencies(${CARGO_NAME}_out_file )

  set_target_properties(
    ${CARGO_NAME}
    PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE
               IMPORTED_NO_SONAME TRUE
               IMPORTED_LOCATION "${OUT_FILE}"
               # rust project dont really have include directories.
               # assume that if we have c-headers. they will be in ./include
               INTERFACE_INCLUDE_DIRECTORIES
               $<BUILD_INTERFACE:${TARGET_SRC}/include>
               INTERFACE_LINK_LIBRARIES "pthread;dl")
endfunction()

# set cargo build arguments params: 
# --locked   only build specific versions
# defined in Cargo.lock file. the goal is to get more deterministic builds
# --target   specify the target triple to build for.
get_target_triple(CARGO_TARGET_TRIPLE)
rust_build_profile(CARGO_PROFILE)
set(CARGO_ARGS
    " --locked --target ${CARGO_TARGET_TRIPLE} --profile ${CARGO_PROFILE}")

set(APP_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REV})

# This function configures a build-process that builds the given manifest_path. 
# Every declaration of this function builds in a separate process. This has some
# performance implications, as dependencies that are required in multiple build
# processes will be build double. Therefore, be cautious with having a fine
# granularity of build processes. It will degrade the build performance.
#
# arguments:
#
# * `NAME` the name of the cargo crate to build
# * `DEPENDS` the cmake targets that need to be build first before the crate
#   can be build successfully
function(cargo_build)
  cmake_parse_arguments(CARGO "DISABLE_TESTS" "NAME;MANIFEST_PATH" "DEPENDS"
                        ${ARGN})
  if(NOT CARGO_MANIFEST_PATH OR NOT CARGO_NAME)
    message(
      FATAL_ERROR
        "${CARGO_MANIFEST_PATH} or ${CARGO_NAME} not set in cargo_build(${ARGN})"
    )
  endif()

  # Get the meta-data corresponding the target. Used to configure the rest of
  # the rust target.
  if(NOT EXISTS ${CARGO_MANIFEST_PATH}/Cargo.toml)
    message(FATAL_ERROR "Did not find cargo manifest in ${CARGO_MANIFEST_PATH}")
  endif()
  rust_workspace_metadata(${CARGO_MANIFEST_PATH} CARGO_META)
  message(DEBUG "Added a rust build-target ${CARGO_NAME}")

  set(CARGO_BIN_DIR ${CMAKE_BINARY_DIR}/cargo/${CARGO_NAME})
  set(${CARGO_NAME}_ENV
      ${CMAKE_COMMAND}
      -E
      env
      "VERSION_STR=${APP_VERSION}"
      "UNIFY_BINARY_DIR=${CMAKE_BINARY_DIR}"
      "CARGO_PROFILE_RELWITHDEBINFO_INHERITS=release"
      "CARGO_PROFILE_RELWITHDEBINFO_DEBUG=1"
      "CARGO_PROFILE_RELWITHDEBINFO_DEBUG_ASSERTIONS=true"
      "CARGO_TARGET_DIR=${CARGO_BIN_DIR}")

  add_custom_target(
    ${CARGO_NAME}_output_dir COMMAND ${CMAKE_COMMAND} -E make_directory
                                     "${CARGO_BIN_DIR}")
  add_custom_target(
    ${CARGO_NAME}
    COMMAND
      ${${CARGO_NAME}_ENV} "bash" "-c"
      "${CARGO_EXECUTABLE} build --manifest-path ${CARGO_MANIFEST_PATH}/Cargo.toml ${CARGO_ARGS}"
    DEPENDS ${CARGO_DEPENDS} ${CARGO_NAME}_output_dir
    COMMENT "Running: cargo build target ${CARGO_NAME}"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    VERBATIM)

  set_property(TARGET ${CARGO_NAME} PROPERTY CARGO_META "${CARGO_META}")
  set_property(TARGET ${CARGO_NAME} PROPERTY CARGO_TARGET_DIR
                                             "${CARGO_BIN_DIR}")
  set_property(TARGET ${CARGO_NAME} PROPERTY CARGO_ENV "${${CARGO_NAME}_ENV}")

  if(BUILD_TESTING AND NOT ${CARGO_DISABLE_TESTS})
    add_test(
      NAME ${CARGO_NAME}
      COMMAND
        ${${CARGO_NAME}_ENV} "bash" "-c"
        "${CARGO_EXECUTABLE} test --manifest-path ${CARGO_MANIFEST_PATH}/Cargo.toml ${CARGO_ARGS}"
    )
  endif()
endfunction()

# With this function you can import rust projects, that are part of the rust
# build process, into cmake. From this point on wards they can be used like any
# other cmake build target. 
function(cargo_as_cmake_target BUILD_TARGET CARGO_NAME)
  if(NOT BUILD_TARGET OR NOT CARGO_NAME)
    message(FATAL_ERROR "missing arguments in cargo_as_cmake_target(${ARGN})")
  endif()

  # get the meta data that corresponds to the workspace in which the requested
  # package resides
  get_target_property(CARGO_META ${BUILD_TARGET} CARGO_META)
  if(NOT CARGO_META)
    message(FATAL_ERROR "a build target ${BUILD_TARGET} is not declared yet
    anywhere")
  endif()

  # loop through the workspace packages and find the JSON related to the
  # cargo_name
  rust_package_meta(${CARGO_META} ${CARGO_NAME} CARGO_META_PACKAGE)
  # add a imported cmake target, so that the component can be used like it as a
  # cmake declared component
  add_cmake_target(${CARGO_META} ${CARGO_META_PACKAGE} ${BUILD_TARGET})
  # make sure the workspace is build in which the package resides
  add_dependencies(${CARGO_NAME} ${BUILD_TARGET})

  rust_is_configured_for_deb(${CARGO_META_PACKAGE} HAS_DEB)
  if(HAS_DEB)
    string(JSON CARGO_PACKAGE_MANIFEST GET ${CARGO_META_PACKAGE}
           "manifest_path")
    get_target_property(CARGO_ENV_COMMAND ${BUILD_TARGET} CARGO_ENV)
    # setup deb package generation. this target is added to the cmake target
    # `deb`
    set(DEB_FILE
        "${CMAKE_BINARY_DIR}/${CARGO_NAME}_${FILE_NAME_VERSIONING_ARCH}.deb")
    add_custom_command(
      OUTPUT ${DEB_FILE}
      COMMAND
        ${CARGO_ENV_COMMAND} ${CARGO_EXECUTABLE} "deb" ARGS "-p" "${CARGO_NAME}"
        "--no-strip" "--no-build" "--deb-version" "${APP_VERSION}" "--output"
        "${DEB_FILE}" "--target" "${CARGO_TARGET_TRIPLE}" "--manifest-path"
        "${CARGO_PACKAGE_MANIFEST}" "--profile" "${CARGO_PROFILE}"
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      DEPENDS ${BUILD_TARGET}
      COMMENT "Running: cargo deb ${CARGO_NAME}")
    add_custom_target(${CARGO_NAME}_deb DEPENDS ${DEB_FILE})
    add_dependencies(deb ${CARGO_NAME}_deb)
  endif()
endfunction()
