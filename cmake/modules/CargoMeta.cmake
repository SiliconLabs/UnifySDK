# Helper function to get the target triple from cmake and get the rust variant
# for it.
function(get_target_triple CARGO_TARGET_TRIPLE)
  if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      set(CARGO_TARGET_TRIPLE
          "x86_64-pc-windows-msvc"
          PARENT_SCOPE)
    else()
      set(CARGO_TARGET_TRIPLE
          "i686-pc-windows-msvc"
          PARENT_SCOPE)
    endif()
  elseif(ANDROID)
    if(ANDROID_SYSROOT_ABI STREQUAL "x86")
      set(CARGO_TARGET_TRIPLE
          "i686-linux-android"
          PARENT_SCOPE)
    elseif(ANDROID_SYSROOT_ABI STREQUAL "x86_64")
      set(CARGO_TARGET_TRIPLE
          "x86_64-linux-android"
          PARENT_SCOPE)
    elseif(ANDROID_SYSROOT_ABI STREQUAL "arm")
      set(CARGO_TARGET_TRIPLE
          "arm-linux-androideabi"
          PARENT_SCOPE)
    elseif(ANDROID_SYSROOT_ABI STREQUAL "arm64")
      set(CARGO_TARGET_TRIPLE
          "aarch64-linux-android"
          PARENT_SCOPE)
    endif()
  elseif(IOS)
    set(CARGO_TARGET_TRIPLE
        "universal"
        PARENT_SCOPE)
  elseif(CMAKE_SYSTEM_NAME STREQUAL Darwin)
    set(CARGO_TARGET_TRIPLE
        "x86_64-apple-darwin"
        PARENT_SCOPE)
  elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "armhf")
    set(CARGO_TARGET_TRIPLE
        "armv7-unknown-linux-gnueabihf"
        PARENT_SCOPE)
  elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
    set(CARGO_TARGET_TRIPLE
        "aarch64-unknown-linux-gnu"
        PARENT_SCOPE)
  else()
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      set(CARGO_TARGET_TRIPLE
          "x86_64-unknown-linux-gnu"
          PARENT_SCOPE)
    else()
      set(CARGO_TARGET_TRIPLE
          "i686-unknown-linux-gnu"
          PARENT_SCOPE)
    endif()
  endif()
endfunction()

function(rust_build_profile CARGO_PROFILE)
  if(CMAKE_BUILD_TYPE MATCHES "Release")
    set(CARGO_PROFILE
        "release"
        PARENT_SCOPE)
  elseif(CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
    set(CARGO_PROFILE
        "RelWithDebInfo"
        PARENT_SCOPE)
  else()
    set(CARGO_PROFILE
        "dev"
        PARENT_SCOPE)
  endif()
endfunction()

function(rust_output_filename CARGO_META_PACKAGE OUT_FILE)
  string(JSON TARGET_NAME GET ${CARGO_META_PACKAGE} "name")
  rust_get_build_type(${CARGO_META_PACKAGE} TARGET_TYPE)
  if(${TARGET_TYPE} STREQUAL "bin")
    set(${OUT_FILE}
        ${TARGET_NAME}
        PARENT_SCOPE)
  elseif(${TARGET_TYPE} STREQUAL "dylib")
    set(${OUT_FILE}
        ${CMAKE_SHARED_LIBRARY_PREFIX}${TARGET_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}
        PARENT_SCOPE)
  elseif(${TARGET_TYPE} STREQUAL "lib")
    set(${OUT_FILE}
        ${CMAKE_STATIC_LIBRARY_PREFIX}${TARGET_NAME}.rlib
        PARENT_SCOPE)
  elseif(${TARGET_TYPE} STREQUAL "staticlib")
    set(${OUT_FILE}
        ${CMAKE_STATIC_LIBRARY_PREFIX}${TARGET_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}
        PARENT_SCOPE)
  endif()
endfunction()

function(rust_native_target_dir CARGO_TARGET_DIR OUT_DIR)
  rust_build_profile(CARGO_PROFILE)
  if(CARGO_PROFILE MATCHES "dev")
    set(CARGO_PROFILE "debug")
  endif()

  set(${OUT_DIR}
      "${CARGO_TARGET_DIR}/${CARGO_TARGET_TRIPLE}/${CARGO_PROFILE}"
      PARENT_SCOPE)
endfunction()

function(rust_native_target_file CARGO_TARGET_DIR CARGO_META_PACKAGE NATIVE_OUT)
  rust_native_target_dir(${CARGO_TARGET_DIR} OUT_DIR)
  rust_output_filename(${CARGO_META_PACKAGE} OUT_FILENAME)
  set(${NATIVE_OUT}
      "${OUT_DIR}/${OUT_FILENAME}"
      PARENT_SCOPE)
endfunction()

function(rust_package_meta CARGO_METADATA CARGO_PACKAGE_NAME PACKAGE_JSON)
  string(
    JSON
    CARGO_PACKAGES
    ERROR_VARIABLE
    PACKAGES_ERROR
    GET
    ${CARGO_METADATA}
    "packages")

  if(PACKAGES_ERROR)
    message(FATAL_ERROR "Error parsing rust meta: ${PACKAGES_ERROR}")
  endif()

  string(JSON CARGO_PACKAGES_LENGTH LENGTH ${CARGO_PACKAGES})
  math(EXPR CARGO_PACKAGES_LENGTH "${CARGO_PACKAGES_LENGTH}-1")
  foreach(IDX RANGE ${CARGO_PACKAGES_LENGTH})
    string(
      JSON
      CARGO_PACKAGE
      ERROR_VARIABLE
      PACKAGES_ERROR
      GET
      ${CARGO_PACKAGES}
      ${IDX})
    if(PACKAGES_ERROR)
      message(FATAL_ERROR "Error parsing rust meta: ${PACKAGES_ERROR}")
    endif()

    string(JSON CARGO_PACKAGE_NAME GET ${CARGO_PACKAGE} "name")
    if(${CARGO_NAME} STREQUAL ${CARGO_PACKAGE_NAME})
      set(${PACKAGE_JSON}
          ${CARGO_PACKAGE}
          PARENT_SCOPE)
      return()
    endif()
  endforeach()
  message(
    FATAL_ERROR
      "could not find meta data for ${CARGO_PACKAGE_NAME}. Did you add the project to the top level Cargo.toml?"
  )
endfunction()

function(rust_get_build_types CARGO_META_PACKAGE CARGO_BUILD_TYPES)
  # assumed is that the primary target is always defined first at index 0
  string(
    JSON
    META_BUILD_TYPES
    GET
    ${CARGO_META_PACKAGE}
    "targets"
    0
    "crate_types")
  set(${CARGO_BUILD_TYPES}
      ${META_BUILD_TYPES}
      PARENT_SCOPE)
endfunction()

function(rust_get_target_src CARGO_META_PACKAGE CARGO_SRC_PATH)
  string(JSON CARGO_TARGET_NAME GET ${CARGO_META_PACKAGE} "name")
  string(JSON CARGO_TARGET_LENGTH LENGTH ${CARGO_META_PACKAGE} "targets")
  math(EXPR CARGO_TARGET_LENGTH "${CARGO_TARGET_LENGTH}-1")
  foreach(IDX RANGE ${CARGO_TARGET_LENGTH})
    string(
      JSON
      META_TARGET_NAME
      GET
      ${CARGO_META_PACKAGE}
      "targets"
      ${IDX}
      "name")
    if(${META_TARGET_NAME} STREQUAL ${CARGO_TARGET_NAME})
      string(
        JSON
        META_BUILD_TYPES
        GET
        ${CARGO_META_PACKAGE}
        "targets"
        ${IDX}
        "src_path")
      set(${CARGO_SRC_PATH}
          ${META_BUILD_TYPES}
          PARENT_SCOPE)
      return()
    endif()
  endforeach()
endfunction()

function(rust_contains_build_type CARGO_META_PACKAGE ELEMENT FOUND)
  rust_get_build_types(${CARGO_META_PACKAGE} CARGO_BUILD_TYPES)
  string(JSON LEN LENGTH ${CARGO_BUILD_TYPES})
  math(EXPR LEN "${LEN}-1")
  foreach(IDX RANGE ${LEN})
    string(JSON I GET ${CARGO_BUILD_TYPES} ${IDX})
    if(${ELEMENT} STREQUAL ${I})
      set(${FOUND}
          true
          PARENT_SCOPE)
      return()
    endif()
  endforeach()
  set(${FOUND}
      false
      PARENT_SCOPE)
endfunction()

# Determine the target type of a provided metadata package. The types are
# checked in order of priority. This means if cargo metadata returns `bin` and
# `dylib` for a given target, bin will be returned as primary target. Use
# `rust_contains_build_type` otherwise to determine if a certain type is
# implemented.
function(rust_get_build_type CARGO_META_PACKAGE TARGET_TYPE)
  rust_contains_build_type(${CARGO_META_PACKAGE} "bin" BINARY)
  if(BINARY)
    set(${TARGET_TYPE}
        "bin"
        PARENT_SCOPE)
    return()
  endif()
  rust_contains_build_type(${CARGO_META_PACKAGE} "staticlib" STATIC_LIB)
  if(STATIC_LIB)
    set(${TARGET_TYPE}
        "staticlib"
        PARENT_SCOPE)
    return()
  endif()
  rust_contains_build_type(${CARGO_META_PACKAGE} "dylib" DYNAMIC_LIB)
  if(DYNAMIC_LIB)
    set(${TARGET_TYPE}
        "dylib"
        PARENT_SCOPE)
    return()
  endif()
  rust_contains_build_type(${CARGO_META_PACKAGE} "lib" RLIB)
  if(RLIB)
    set(${TARGET_TYPE}
        "lib"
        PARENT_SCOPE)
    return()
  endif()
  rust_contains_build_type(${CARGO_META_PACKAGE} "proc-macro" PROC)
  if(PROC)
    set(${TARGET_TYPE}
        "proc-macro"
        PARENT_SCOPE)
    return()
  endif()

  string(JSON TARGET_NAME GET ${CARGO_META_PACKAGE} "name")
  rust_get_build_types(${CARGO_META_PACKAGE} CARGO_BUILD_TYPES)
  message(
    FATAL_ERROR
      "Could not deduct target type from ${CARGO_BUILD_TYPES} for ${TARGET_NAME}"
  )
endfunction()

function(rust_source_path CARGO_META_PACKAGE SOURCE_PATH)
  string(JSON manifest_path GET ${CARGO_META_PACKAGE} "manifest_path")
  get_filename_component(base_path ${manifest_path} DIRECTORY)
  set(${SOURCE_PATH}
      ${base_path}
      PARENT_SCOPE)
endfunction()

# get the source_path directories of the current meta package and all child
# dependencies
function(rust_source_paths CARGO_META_PACKAGE SOURCE_PATHS)
  rust_source_path(${CARGO_META_PACKAGE} CURRENT_PATH)
  set(DEP_PATHS ${CURRENT_PATH})

  # append all paths of children
  list(APPEND SOURCE_PATHS ${DEP_PATH} PARENT_SCOPE)
  string(JSON LEN LENGTH ${CARGO_META_PACKAGE} "dependencies")
  math(EXPR LEN "${LEN}-1")
  foreach(IDX RANGE ${LEN})
    string(
      JSON
      DEP_PATH
      ERROR_VARIABLE
      NO_PATH
      GET
      ${CARGO_META_PACKAGE}
      "dependencies"
      ${IDX}
      "path")
    if(NOT NO_PATH)
      list(APPEND DEP_PATHS ${DEP_PATH})
    endif()
  endforeach()
  set(SOURCE_PATHS
      ${DEP_PATHS}
      PARENT_SCOPE)
endfunction()

function(rust_rel_output_dir CARGO_META_PACKAGE OUT_DIR)
  string(JSON manifest_path GET ${CARGO_META_PACKAGE} "manifest_path")
  get_filename_component(base_path ${manifest_path} DIRECTORY)
  file(RELATIVE_PATH COMPONENT_PATH ${PROJECT_SOURCE_DIR} ${base_path})
  set(${OUT_DIR}
      ${COMPONENT_PATH}
      PARENT_SCOPE)
endfunction()

function(rust_glob_sources CARGO_META_PACKAGE RUST_SOURCES)
  rust_source_paths(${CARGO_META_PACKAGE} SOURCE_PATHS)
  foreach(SOURCE_PATH ${SOURCE_PATHS})
    if(EXISTS ${SOURCE_PATH}/Cargo.toml)
      list(APPEND SOURCES ${SOURCE_PATH}/Cargo.toml)
    endif()

    if(EXISTS ${SOURCE_PATH}/Cargo.lock)
      list(APPEND SOURCES ${SOURCE_PATH}/Cargo.lock)
    endif()

    file(GLOB_RECURSE GLOBBED "${SOURCE_PATH}/*.rs")
    list(APPEND SOURCES ${GLOBBED})
  endforeach()
  set(${RUST_SOURCES}
      ${SOURCES}
      PARENT_SCOPE)
endfunction()

function(rust_is_configured_for_deb CARGO_META_PACKAGE CONFIGURED)
  set(${CONFIGURED}
      false
      PARENT_SCOPE)
  string(
    JSON
    RESULT
    ERROR_VARIABLE
    ERR
    GET
    ${CARGO_META_PACKAGE}
    "metadata"
    "deb")
  if(NOT ERR)
    set(${CONFIGURED}
        true
        PARENT_SCOPE)
  endif()
endfunction()

get_target_triple(CARGO_TARGET_TRIPLE)
set(DIR_OF_CARGO_META
    ${CMAKE_CURRENT_LIST_DIR}
    CACHE INTERNAL "DIR_OF_CARGO_META")

function(rust_workspace_metadata CARGO_MANIFEST_DIR CARGO_METADATA)
  if(CMAKE_Rust_COMPILER)
    execute_process(
      COMMAND
        ${CARGO_EXECUTABLE} "metadata" "--manifest-path"
        "${CARGO_MANIFEST_DIR}/Cargo.toml" "--format-version" "1"
        "--filter-platform" "${CARGO_TARGET_TRIPLE}"
      OUTPUT_VARIABLE METADATA COMMAND_ERROR_IS_FATAL ANY)
    set(${CARGO_METADATA}
        ${METADATA}
        PARENT_SCOPE)
  endif()
endfunction()
