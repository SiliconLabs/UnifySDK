include(CargoMeta)
include(RustBindingGenerator)

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
#    environment variable or configuration will trigger an rebuild. more about
#    this in Cargo Fingerprinting.
# 2. Cargo manages a lot of build configuration. based on experience we saw that
#    its very hard to inject custom configuration via additional flags without
#    it conflicted or even simply being ignored. Rust builds are dependant on
#    all native cpp/c dependencies. This means before the start all native cpp/c
#    libraries are build.
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
#   this means you can integrate it easy into other cmake targets.
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
# cmake target: <rust_name>_target. note: dont make cmake target names that are
# equal to the imported location filename or output file.
function(add_cmake_target CARGO_META_PACKAGE)
  string(JSON CARGO_TARGET_NAME GET ${CARGO_META_PACKAGE} "name")
  rust_native_target_file(${CARGO_META_PACKAGE} OUT_FILE)
  rust_source_path(${CARGO_META_PACKAGE} TARGET_SRC)

  rust_get_build_type(${CARGO_META_PACKAGE} TARGET_TYPE)
  if(${TARGET_TYPE} STREQUAL "bin")
    add_executable(${CARGO_TARGET_NAME}_target IMPORTED GLOBAL)
  elseif(${TARGET_TYPE} STREQUAL "dylib" OR ${TARGET_TYPE} STREQUAL "lib")
    add_library(${CARGO_TARGET_NAME}_target SHARED IMPORTED GLOBAL)
  elseif(${TARGET_TYPE} STREQUAL "staticlib")
    add_library(${CARGO_TARGET_NAME}_target STATIC IMPORTED GLOBAL)
    # an attempt to add link libraries for a rust target.
    set_target_properties(${CARGO_TARGET_NAME}_target
                          PROPERTIES INTERFACE_LINK_LIBRARIES "pthread;dl")
  endif()

  # workaround, a cmake target cant have a dependency on file ?!, therefore
  # create an intermediate target <target>_fwd and let the cmake target depend
  # on that
  add_custom_target(${CARGO_TARGET_NAME}_fwd DEPENDS "${OUT_FILE}")
  add_dependencies(${CARGO_TARGET_NAME}_target ${CARGO_TARGET_NAME}_fwd)

  set_target_properties(
    ${CARGO_TARGET_NAME}_target
    PROPERTIES IMPORTED_LOCATION ${OUT_FILE}
               # rust project dont really have include directories.
               # for now, just set the root folder of the rust project.
               INCLUDE_DIRECTORIES ${TARGET_SRC}
               WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endfunction()

function(rust_add_native_libs CARGO_DEPENDS)
  list(APPEND new_libs ${CARGO_DEPENDS})
  get_target_property(NATIVE_LIBS cargo_build NATIVE_LIBS)
  if(NATIVE_LIBS)
    list(APPEND new_libs ${NATIVE_LIBS})
  endif()
  set_target_properties(cargo_build PROPERTIES NATIVE_LIBS "${new_libs}")

  foreach(ll ${CARGO_DEPENDS})
    list(APPEND idirs "$<TARGET_PROPERTY:${ll},INCLUDE_DIRECTORIES>")
    get_target_property(TARGET_DIR ${ll} BINARY_DIR)
    list(APPEND new_search_paths ${TARGET_DIR})
  endforeach()

  get_target_property(includes cargo_build INTERFACE_INCLUDE_DIRECTORIES)
  list(APPEND includes ${idirs})
  set_target_properties(cargo_build PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
                                               "${includes}")

  get_target_property(SEARCH_PATHS cargo_build SEARCH_PATHS)
  if(SEARCH_PATHS)
    list(APPEND new_search_paths ${SEARCH_PATHS})
  endif()
  set_target_properties(cargo_build PROPERTIES SEARCH_PATHS
                                               "${new_search_paths}")
endfunction()

function(rust_add_package_for_build NAME)
  get_target_property(packages cargo_build BUILD_PACKAGES)
  if(packages)
    list(APPEND packages "${NAME}")
  else()
    set(packages "${NAME}")
  endif()
  set_target_properties(cargo_build PROPERTIES BUILD_PACKAGES "${packages}")
endfunction()

function(build_rustflags RUSTFLAGS)
  # warnings as errors
  string(APPEND ARGS " --deny warnings ")

  # search paths for where to find these libs
  string(
    APPEND
    ARGS
    " -L native=$<JOIN:$<REMOVE_DUPLICATES:$<TARGET_PROPERTY:cargo_build,SEARCH_PATHS>>, -L native=>"
  )

  # do not export local file directories into executables in case of non debug
  # build
  if(NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE MATCHES Debug)
    # add these searchpaths to the binaries to let the dynamic linker know where
    # to find the libs
    string(
      APPEND
      ARGS
      " -Clink-arg=-Wl,-rpath,$<JOIN:$<REMOVE_DUPLICATES:$<TARGET_PROPERTY:cargo_build,SEARCH_PATHS>>, -Clink-arg=-Wl,-rpath,>"
    )
  endif()
  set(${RUSTFLAGS}
      "${ARGS}"
      PARENT_SCOPE)
endfunction()

# set cargo build arguments params: --locked    only build specific versions
# defined in Cargo.lock file. the goal is to get more deterministic builds
# --target    specify the target triple to build for.
get_target_triple(CARGO_TARGET_TRIPLE)
set(CARGO_ARGS "--release --locked --target ${CARGO_TARGET_TRIPLE}")

if(APPLE)
  set(LD_RUNTIME "DYLD_FALLBACK_LIBRARY_PATH")
else()
  set(LD_RUNTIME "LD_LIBRARY_PATH")
endif()

# set rust environment - improvememt: refactor the version_str out the
# environment - via RUSTFLAGS all native libraries get passed.
build_rustflags(RUST_FLAGS)

# TODO Figure out a way to get rid of theese hardcoded flags. UIC-1460
#
# set(HARD_CODED_FLAGS "-ldl -lpthread ")

# add cpp symbols. otherwise native libraries using c++lib functions will not be
# able to link.
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  string(APPEND HARD_CODED_FLAGS " -l dylib=c++")
else()
  string(APPEND HARD_CODED_FLAGS " -l dylib=stdc++")
endif()

# Add linking with GCOV in case of gcov being enabled
if(CMAKE_GCOV)
  string(APPEND HARD_CODED_FLAGS " -lgcov")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  string(APPEND ARGS
         " -Clink-arg=-Wl,--unresolved-symbols=ignore-in-object-files")
endif()

set(APP_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REV})

set(CARGO_ENV_COMMAND
    ${CMAKE_COMMAND} -E env "VERSION_STR=${APP_VERSION}"
    "RUSTFLAGS=${HARD_CODED_FLAGS} ${RUST_FLAGS}" "UIC_OUT=${CMAKE_BINARY_DIR}"
    "${LD_RUNTIME}=$<JOIN:$<TARGET_PROPERTY:cargo_build,SEARCH_PATHS>,:>")

# write environment to a file. developers can source this file to their local
# shell, so they can execute cargo commands themselves.
file(
  GENERATE
  OUTPUT ${CMAKE_BINARY_DIR}/unify_env.source
  CONTENT
    "#!/bin/sh
    export VERSION_STR=\"${APP_VERSION}\"
    export UIC_OUT=\"${CMAKE_BINARY_DIR}\"
    export ${LD_RUNTIME}=\"$<JOIN:$<TARGET_PROPERTY:cargo_build,SEARCH_PATHS>,:>\"
    export RUSTFLAGS=\"${HARD_CODED_FLAGS} ${RUST_FLAGS}\"")

set(CARGO_PACKAGES
    "-p $<JOIN:$<TARGET_PROPERTY:cargo_build,BUILD_PACKAGES>, -p >")

# cmake-format: off
# This is the main build target command of all rust components. All declared
# rust targets are build here. it depends on the bindings files to be generated
# + the native C libraries to be there. the cargo_build target is also used to
# bookkeep data. which are stored as properties of `cargo_build`. currently the
# following properties exist: 
# - NATIVE_LIBRARIES      a list with all the c libraries needed for a
#                         successfull rust build.
# - SEARCH_PATH           a list of all paths to these native c libraries.
# - CARGO_LIBS            a list with annotated native libaries, similar to
#                         NATIVE_LIBRARIES, only with KIND=${LIB} prepended
#                         used to serve rustc compiler .
# - BUILD_PACKAGES        a list of rust packages to build. passed to cargo
#                         build.
# cmake-format: on
add_custom_target(
  cargo_build
  COMMAND ${CARGO_ENV_COMMAND} "bash" "-c"
          "${CARGO_EXECUTABLE} build ${CARGO_PACKAGES} ${CARGO_ARGS}"
  DEPENDS rust_bindgen "$<TARGET_PROPERTY:cargo_build,NATIVE_LIBS>"
  COMMENT "Running: cargo build"
  VERBATIM)

add_custom_target(
  cargo_test
  COMMAND ${CARGO_ENV_COMMAND} "bash" "-c"
          "${CARGO_EXECUTABLE} test ${CARGO_PACKAGES} ${CARGO_ARGS}"
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/target
  DEPENDS rust_bindgen "$<TARGET_PROPERTY:cargo_build,NATIVE_LIBS>"
  COMMENT "Running: cargo test")

add_custom_target(
  cargo_doc
  COMMAND ${Python3_EXECUTABLE} ../scripts/build/validate_rust_source_code_leaks.py
  COMMAND ${CARGO_ENV_COMMAND} "bash" "-c"
          "${CARGO_EXECUTABLE} doc --no-deps ${CARGO_PACKAGES} ${CARGO_ARGS}"
  COMMAND
    "ln" "-f" "-s" "${PROJECT_SOURCE_DIR}/target/${CARGO_TARGET_TRIPLE}/doc"
    "${CMAKE_BINARY_DIR}/doc"
  COMMAND
    "zip" "-r" "-q"
    "${CMAKE_BINARY_DIR}/rust_doc_${CMAKE_PROJECT_VERSION}.docs.zip" "doc/*"
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  DEPENDS rust_bindgen "$<TARGET_PROPERTY:cargo_build,NATIVE_LIBS>"
  COMMENT "Running: cargo doc")
add_dependencies(doxygen_zip cargo_doc)

function(cargo_build)
  cmake_parse_arguments(CARGO "" "MANIFEST_FILE" "" "" ${ARGN})
  if(NOT CARGO_MANIFEST_FILE)
    message(FATAL_ERROR "MANIFEST_FILE not set in cargo_build(${ARGN})")
  endif()

  # Get the meta-data corresponding the target. Used to configure the rest of
  # the rust target.
  rust_target_meta(${CARGO_MANIFEST_FILE} CARGO_META_PACKAGE)
  string(JSON CARGO_NAME GET ${CARGO_META_PACKAGE} "name")
  string(JSON CARGO_PACKAGE_MANIFEST GET ${CARGO_META_PACKAGE} "manifest_path")
  message(DEBUG "Adding rust project ${CARGO_NAME} to cmake")

  rust_glob_sources(${CARGO_META_PACKAGE} RUST_SOURCES)
  rust_native_target_file(${CARGO_META_PACKAGE} NATIVE_FILE)
  rust_source_path(${CARGO_META_PACKAGE} TARGET_SRC)
  add_custom_target(${CARGO_NAME}_sources DEPENDS ${RUST_SOURCES})
  add_dependencies(cargo_build ${CARGO_NAME}_sources)
  add_dependencies(cargo_test ${CARGO_NAME}_sources)

  # we build all rust targets with one cargo command. However to facilitate the
  # permutations builds we need to be able to switch on/off packages. add them
  # here, the actuall packages will be expanded in the actual cargo_build
  # command.
  rust_add_package_for_build(${CARGO_NAME})

  # in the case of a library, add a post step to remove any native symbols that
  # where linked in.
  add_custom_command(OUTPUT "${NATIVE_FILE}" DEPENDS cargo_build)

  add_cmake_target(${CARGO_META_PACKAGE})

  rust_is_configured_for_deb(${CARGO_META_PACKAGE} HAS_DEB)
  if(HAS_DEB)
    # setup deb package generation. this target is added to the cmake target
    # `deb`
    set(DEB_FILE
        "${CMAKE_BINARY_DIR}/${CARGO_NAME}_${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}.${CMAKE_PROJECT_VERSION_PATCH}_${CMAKE_SYSTEM_PROCESSOR}.deb"
    )
    add_custom_command(
      OUTPUT ${DEB_FILE}
      # Creating symlink to debconf directory is workaround for
      # https://github.com/mmstick/cargo-deb/issues/151 Without this manually
      # created maintainer scripts (except service) will be replaced with
      # autogenerated scripts in *.deb packages
      COMMAND "ln" "-f" "-s" "${TARGET_SRC}/debconf"
              "${CMAKE_CURRENT_BINARY_DIR}/debconf"
      COMMAND
        ${CARGO_ENV_COMMAND} ${CARGO_EXECUTABLE} "deb" ARGS "-p" "${CARGO_NAME}"
        "--no-strip" "--no-build" "--deb-version" "${APP_VERSION}" "--output"
        "${DEB_FILE}" "--target" "${CARGO_TARGET_TRIPLE}" "--manifest-path"
        "${CARGO_PACKAGE_MANIFEST}"
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      DEPENDS "${CARGO_NAME}_target" "${TARGET_SRC}/Cargo.toml"
      COMMENT "Running: cargo deb
${CARGO_NAME}")
    add_custom_target(${CARGO_NAME}_deb DEPENDS ${DEB_FILE})
    add_dependencies(deb ${CARGO_NAME}_deb)
  endif()
endfunction()
