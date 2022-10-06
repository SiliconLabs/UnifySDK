if(NOT CMAKE_CARGO_INCLUDED)
  set(CMAKE_CARGO_INCLUDED True)
else()
  return()
endif()

include(CargoMeta)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(Python3 REQUIRED)

# cmake-format: off
# This file specifies the build of rust packages. For usage, skip to function
# `cargo_build`. The goal designing these scripts was to be able to build rust
# packages as fast as possible. We try to achieve this by having as less as
# custom targets as possible. And let cargo do as much as buildsystem work for
# us. "Dont fight the cargo buildsystem!"
# 1. Cargo is quite aggressive in flagging packages dirty, a slight change in an
# environment variable or configuration will trigger an rebuild. more about
# this in Cargo Fingerprinting.
# 2. Cargo manages a lot of build configuration. based on experience we saw that
# its very hard to inject custom configuration via additional flags without
# it conflicted or even simply being ignored. Rust builds are dependant on
# all native cpp/c dependencies. This means before the start all native cpp/c
# libraries are build.
#
# Building of these packages is done in several steps.
# 1. All Rust binaries and libs are build -> cmake target: cargo_build
# 2. All Rust tests are build             -> cmake target: cargo_test
# 3. All Rust doc's are build             -> cmake target: cargo_doc
#
# In theory you dont have to do anything in order for your new rust package
# to get build as Cargo will handle this for us. However you still might want
# to declare your rust packages with `cargo_build` as it enables more
# features:
# - export of your rust package as actual cmake targets. see `add_cmake_target`
# this means you can integrate it easy into other cmake targets.
# - debian package generation.
# - building a single rust package.
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
# hash is based on various things including rust compiler version, rustc binary,
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
# # ${PROJECT_SOURCE_DIR}/target
# rust binaries are build in the target folder rather then the cmake build
# output folder. Again to keep cargo default behavior. The rust build jobs will
# symlink binaries to the cmake build output for backwards compatibility
# reasons.
# cmake-format: on

# this function declares a cmake target for a given rust target. name of the
# cmake target: <rust_name>. note: dont make cmake target names that are equal
# to the imported location filename or output file.
function(add_cmake_target CARGO_META_PACKAGE)
  string(JSON CARGO_TARGET_NAME GET ${CARGO_META_PACKAGE} "name")
  rust_native_target_file(${CARGO_META_PACKAGE} OUT_FILE)
  rust_source_path(${CARGO_META_PACKAGE} TARGET_SRC)
  rust_get_build_type(${CARGO_META_PACKAGE} TARGET_TYPE)

  if(${TARGET_TYPE} STREQUAL "bin")
	  #add_executable(${CARGO_TARGET_NAME} IMPORTED GLOBAL)
    return()
  elseif(${TARGET_TYPE} STREQUAL "dylib" OR ${TARGET_TYPE} STREQUAL "lib")
    add_library(${CARGO_TARGET_NAME} SHARED IMPORTED GLOBAL)
  elseif(${TARGET_TYPE} STREQUAL "staticlib")
    add_library(${CARGO_TARGET_NAME} STATIC IMPORTED GLOBAL)
  endif()

  add_custom_target(
    ${CARGO_TARGET_NAME}_dummy
    COMMAND ${CMAKE_COMMAND} -E touch "${OUT_FILE}"
    DEPENDS "${OUT_FILE}")

  set_target_properties(
    ${CARGO_TARGET_NAME}
    PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE
    IMPORTED_NO_SONAME TRUE
    IMPORTED_LOCATION "${OUT_FILE}"

    # rust project dont really have include directories.
    # assume that if we have c-headers. they will be in ./include
    INCLUDE_DIRECTORIES ${TARGET_SRC}/include
    INTERFACE_LINK_LIBRARIES "pthread;dl")

  # RPath propagation is budged! Workaround: Explicitly set it here so it will
  # be picked up by cmake
  rust_native_target_dir(${CARGO_META_PACKAGE} RUST_LIB_DIR)
  list(APPEND CMAKE_INSTALL_RPATH "${RUST_LIB_DIR}")
  set(CMAKE_MACOSX_RPATH 1)
endfunction()

# set cargo build arguments params: --locked    only build specific versions
# defined in Cargo.lock file. the goal is to get more deterministic builds
# --target    specify the target triple to build for.
get_target_triple(CARGO_TARGET_TRIPLE)
rust_build_profile(CARGO_PROFILE)
set(CARGO_ARGS
  " --locked --target ${CARGO_TARGET_TRIPLE} --profile ${CARGO_PROFILE}")

set(APP_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REV})

# this variable sets the folder in which cargo outputs its build files
set(CARGO_TARGET_DIR "${CMAKE_BINARY_DIR}/cargo")

set(CARGO_ENV_COMMAND
  ${CMAKE_COMMAND} -E env "VERSION_STR=${APP_VERSION}"
  "UNIFY_BINARY_DIR=${CMAKE_BINARY_DIR}" 
  "CARGO_TARGET_DIR=${CARGO_TARGET_DIR}")

if(CMAKE_GCOV)
  list(APPEND CARGO_ENV_COMMAND "RUSTFLAGS=-lgcov")
endif()


add_custom_target(
  cargo_doc
  COMMAND ${Python3_EXECUTABLE}
  ${CMAKE_CURRENT_LIST_DIR}/../../scripts/build/validate_rust_source_code_leaks.py
  COMMAND ${CARGO_ENV_COMMAND} "bash" "-c"
  "${CARGO_EXECUTABLE} doc --no-deps ${CARGO_PACKAGES} ${CARGO_ARGS}"
  COMMAND
  "zip" "-r" "-q"
  "${CMAKE_BINARY_DIR}/rust_doc_${FILE_NAME_VERSIONING}.docs.zip"
  "${CARGO_TARGET_DIR}/${CARGO_TARGET_TRIPLE}/doc/*"
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMENT "Running: cargo doc")

if(DOXYGEN_FOUND)
  add_dependencies(doxygen_zip cargo_doc)
endif()

add_custom_target(cargo_output_dir COMMAND ${CMAKE_COMMAND} -E make_directory
  "${CARGO_TARGET_DIR}")

function(cargo_build)
  cmake_parse_arguments(CARGO "" "NAME" "DEPENDS" ${ARGN})
  if(NOT CARGO_NAME)
    message(FATAL_ERROR "NAME not set in cargo_build(${ARGN})")
  endif()

  # Get the meta-data corresponding the target. Used to configure the rest of
  # the rust target.
  rust_target_meta(${CARGO_NAME} CARGO_META_PACKAGE)
  string(JSON CARGO_PACKAGE_MANIFEST GET ${CARGO_META_PACKAGE} "manifest_path")
  message(DEBUG "Adding rust project ${CARGO_NAME} to cmake")

  # adds a cmake 

  rust_glob_sources(${CARGO_META_PACKAGE} RUST_SOURCES)
  rust_native_target_file(${CARGO_META_PACKAGE} OUT_FILE)
  add_custom_command(
    OUTPUT ${OUT_FILE}
    COMMAND ${CARGO_ENV_COMMAND} "bash" "-c"
    "${CARGO_EXECUTABLE} build -p ${CARGO_NAME} ${CARGO_ARGS}"
    DEPENDS ${RUST_SOURCES} "${CARGO_DEPENDS}"
    cargo_output_dir
    COMMENT "Running: cargo build for ${CARGO_NAME}"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    VERBATIM)
  add_cmake_target(${CARGO_META_PACKAGE})

  # make sure that the the cargo package is build in cmake's ALL
  # target.
  add_custom_target(${CARGO_NAME}_build ALL
	  DEPENDS "${OUT_FILE}")

  rust_is_configured_for_deb(${CARGO_META_PACKAGE} HAS_DEB)
  if(HAS_DEB)
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
      DEPENDS ${CARGO_NAME}_build
      COMMENT "Running: cargo deb ${CARGO_NAME}")
    add_custom_target(${CARGO_NAME}_deb DEPENDS ${DEB_FILE})
    add_dependencies(deb ${CARGO_NAME}_deb)
  endif()

  add_test(NAME ${CARGO_NAME}_test
    COMMAND ${CARGO_ENV_COMMAND} "bash" "-c"
    "${CARGO_EXECUTABLE} test -p ${CARGO_NAME} ${CARGO_ARGS}"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endfunction()

