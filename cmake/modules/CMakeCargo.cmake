# Shamelessly (and legally) borrowed from
# https://github.com/Devolutions/CMakeRust/
function(cargo_print)
  execute_process(COMMAND ${CMAKE_COMMAND} -E echo "${ARGN}")
endfunction()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Create target for all cargo tests
add_custom_target(cargo_test_all)
add_custom_target(cargo_test_report_all)

# determine libuic_dir. Buildservers can decide to redirect the output path for
# rust components. Therefore make sure the LIBUIC_DIR set in the environment is
# leading.
if(DEFINED ENV{LIBUIC_DIR})
  set(uic_dir $ENV{LIBUIC_DIR})
else()
  set(uic_dir ${CMAKE_BINARY_DIR}/components)
endif()

# This function handles the following functionality:
#
# * creates a cmake build target for the specified target. cmake target name:
#   <target_name>_target.
# * creates a cmake test target. cmake target name: <target_name>_test.
# * creates a cmake test rapport target. cmake target name:
#   <target_name>_test_report.
# * creates a cmake doc target. cmake target name: <target_name>_doc_target.
#
# Rust targets can be configured in 2 flavors, as executables (default) or as
# library.
#
# With passing the "LIB" flag instructs this function to build a library type.
# The actual native library type is set inside of the Cargo.toml of the
# corresponding crate. Couple of important notes!:
#
# * Static libraries only are supported at the moment.
# * C/C++ native dependencies are omitted in the produced library.
#
# Implementation note: cargo is not able to build specific libraries in
# isolation. this means that if 3 library builds are defined, cargo is called 3
# times to build *all* libraries. As is looks is the additional build cost
# minimal. cargo defers building if nothing changed. Though it can introduce
# some unwanted side effects. think of build steps that write and read generated
# files. the same happens for cargo test and doc targets.
#
# possible arguments:
# cmake-format: off
# +-----------+----------------------------------------------------------------------+----------+
# | Argument  |                             Description                              | optional |
# +-----------+----------------------------------------------------------------------+----------+
# | NAME      | Target name                                                          |          |
# | LIB       | Flag to specify the library output type. not specifying means a      | x        |
# |           | executable is build                                                  |          |
# | EXT_LIB   | List of cmake targets this target depends on                         | x        |
# | INTO      | Merge library with specified library                                 | x        |
# +-----------+----------------------------------------------------------------------+----------+
# cmake-format: on
function(cargo_build)
  cmake_parse_arguments(CARGO "LIB" "NAME;SOURCE_DIR;WORKING_DIRECTORY"
                        "EXT_LIB;INTO" "" ${ARGN})
  string(REPLACE "-" "_" LIB_NAME ${CARGO_NAME})

  # export linkage configuration
  export_target_json(${LIB_NAME} CUSTOM EXT_LIB ${CARGO_EXT_LIB})

  if(NOT DEFINED CARGO_SOURCE_DIR)
    set(CARGO_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
  endif()
  if(NOT DEFINED CARGO_WORKING_DIRECTORY)
    set(CARGO_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  endif()

  if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      set(LIB_TARGET "x86_64-pc-windows-msvc")
    else()
      set(LIB_TARGET "i686-pc-windows-msvc")
    endif()
  elseif(ANDROID)
    if(ANDROID_SYSROOT_ABI STREQUAL "x86")
      set(LIB_TARGET "i686-linux-android")
    elseif(ANDROID_SYSROOT_ABI STREQUAL "x86_64")
      set(LIB_TARGET "x86_64-linux-android")
    elseif(ANDROID_SYSROOT_ABI STREQUAL "arm")
      set(LIB_TARGET "arm-linux-androideabi")
    elseif(ANDROID_SYSROOT_ABI STREQUAL "arm64")
      set(LIB_TARGET "aarch64-linux-android")
    endif()
  elseif(IOS)
    set(LIB_TARGET "universal")
  elseif(CMAKE_SYSTEM_NAME STREQUAL Darwin)
    set(LIB_TARGET "x86_64-apple-darwin")
  elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "armhf")
    set(LIB_TARGET "armv7-unknown-linux-gnueabihf")
  else()
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      set(LIB_TARGET "x86_64-unknown-linux-gnu")
    else()
      set(LIB_TARGET "i686-unknown-linux-gnu")
    endif()
  endif()

  if(NOT CMAKE_BUILD_TYPE)
    set(LIB_BUILD_TYPE "debug")
  elseif(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    set(LIB_BUILD_TYPE "release")
  else()
    set(LIB_BUILD_TYPE "debug")
  endif()

  set(TARGET_OUTPUT
      "${CMAKE_CURRENT_BINARY_DIR}/${LIB_TARGET}/${LIB_BUILD_TYPE}")
  set(TARGET_EXECUTABLE ${TARGET_OUTPUT}/${CARGO_NAME})
  set(LIB_FILE
      "${TARGET_OUTPUT}/${CMAKE_STATIC_LIBRARY_PREFIX}${LIB_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}"
  )

  set(TEST_OUTPUT_FILE
      "${CMAKE_BINARY_DIR}/Testing/${CARGO_NAME}/Test_rust.xml")
  set(DOC_FILE "${CMAKE_BINARY_DIR}/doc_${CARGO_NAME}/${LIB_NAME}/index.hml")
  set(DEB_FILE
      "${CMAKE_BINARY_DIR}/${CARGO_NAME}_${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}.${CMAKE_PROJECT_VERSION_PATCH}_${CMAKE_SYSTEM_PROCESSOR}.deb"
  )

  set(CARGO_ARGS "--target" ${LIB_TARGET})
  if(${LIB_BUILD_TYPE} STREQUAL "release")
    list(APPEND CARGO_ARGS "--release")
  endif()
  file(GLOB_RECURSE LIB_SOURCES "${CARGO_SOURCE_DIR}/*.rs")

  set(APP_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REV})
  set(CARGO_ENV_COMMAND
      ${CMAKE_COMMAND} -E env "CARGO_TARGET_DIR=${CMAKE_CURRENT_BINARY_DIR}"
      "VERSION_STR=${APP_VERSION}" "LIBUIC_DIR=${uic_dir}")

  if(CARGO_LIB)
    set(TARGET_TYPE "--lib")
    add_custom_command(
      OUTPUT ${LIB_FILE}
      COMMAND ${CARGO_ENV_COMMAND} ${CARGO_EXECUTABLE} ARGS build ${CARGO_ARGS}
              ${TARGET_TYPE}
      COMMAND
        CMAKE_AR=${CMAKE_AR} CMAKE_RANLIB=${CMAKE_RANLIB}
        ${PROJECT_SOURCE_DIR}/scripts/build/rust_strip_cxx_from_library.sh
        ${LIB_FILE}
      WORKING_DIRECTORY ${CARGO_WORKING_DIRECTORY}
      DEPENDS ${LIB_SOURCES} ${CARGO_EXT_LIB}
      COMMENT "Running: cargo build ${CARGO_NAME}")
    add_custom_target(${CARGO_NAME}_target ALL DEPENDS ${LIB_FILE})
    add_library(${CARGO_NAME}_lib STATIC IMPORTED GLOBAL)
    add_dependencies(${CARGO_NAME}_lib ${CARGO_NAME}_target)

    set_target_properties(${CARGO_NAME}_lib 
      PROPERTIES IMPORTED_LOCATION ${LIB_FILE}
      INTERFACE_INCLUDE_DIRECTORIES "")

    if (CARGO_INTO)
      foreach(target ${CARGO_INTO})
        add_dependencies(${target} ${CARGO_NAME}_lib)
        target_link_libraries(${target} PRIVATE Threads::Threads dl)
        get_target_property(target_type ${target} TYPE)
        if (target_type MATCHES "STATIC_LIBRARY" OR target_type MATCHES "SHARED_LIBRARY")
          add_custom_command(TARGET ${target} POST_BUILD
            COMMAND CMAKE_AR=${CMAKE_AR} 
                ${PROJECT_SOURCE_DIR}/scripts/build/merge_libs.sh $<TARGET_FILE:${target}> ${LIB_FILE})
        else()
          message(WARNING "wrong target type for ${target}. cannot import ${CARGO_NAME}_target into ${target_type}")
        endif()
      endforeach()
    endif()
  else()
    set(TARGET_TYPE "--bin" ${CARGO_NAME})
    add_custom_target(${CARGO_NAME}_target ALL DEPENDS ${TARGET_EXECUTABLE})
    add_custom_command(
      OUTPUT ${TARGET_EXECUTABLE}
      COMMAND ${CARGO_ENV_COMMAND} ${CARGO_EXECUTABLE} ARGS build ${CARGO_ARGS}
              ${TARGET_TYPE}
      WORKING_DIRECTORY ${CARGO_WORKING_DIRECTORY}
      DEPENDS ${LIB_SOURCES} ${CARGO_EXT_LIB}
      COMMENT "Running: cargo build ${CARGO_NAME}")
  endif()

  # Create target ${CARGO_NAME}_test to run cargo test on the target and output
  # to stdout
  add_custom_target(${CARGO_NAME}_test)
  add_custom_command(
    TARGET ${CARGO_NAME}_test
    COMMAND ${CARGO_ENV_COMMAND} ${CARGO_EXECUTABLE} test ${CARGO_ARGS}
            ${TARGET_TYPE}
    WORKING_DIRECTORY ${CARGO_WORKING_DIRECTORY}
    DEPENDS ${LIB_SOURCES} ${CARGO_EXT_LIB}
    COMMENT "Running: cargo test ${CARGO_NAME}")
  add_dependencies(cargo_test_all ${CARGO_NAME}_test)

  # Create target ${CARGO_NAME}_test to run cargo test on the target and output
  # junit report
  add_custom_target(${CARGO_NAME}_test_report)
  add_custom_command(
    TARGET ${CARGO_NAME}_test_report
    BYPRODUCTS ${TEST_OUTPUT_FILE}
    COMMAND ${CMAKE_COMMAND} -E make_directory
            ${CMAKE_BINARY_DIR}/Testing/${CARGO_NAME}
    COMMAND
      ${CARGO_ENV_COMMAND} ${CARGO_EXECUTABLE} test ${CARGO_ARGS} ${TARGET_TYPE}
      -- -Z unstable-options --format json | cargo2junit > ${TEST_OUTPUT_FILE}
    WORKING_DIRECTORY ${CARGO_WORKING_DIRECTORY}
    DEPENDS ${LIB_SOURCES} ${CARGO_EXT_LIB}
    COMMENT "Running: cargo test report ${CARGO_NAME}")
  add_dependencies(cargo_test_report_all ${CARGO_NAME}_test_report)

  if (CARGO_EXT_LIB)
    add_dependencies(${CARGO_NAME}_test ${CARGO_EXT_LIB})
  add_dependencies(${CARGO_NAME}_test_report ${CARGO_EXT_LIB})
  endif()

  add_custom_command(
    OUTPUT ${DOC_FILE}
    COMMAND ${CARGO_ENV_COMMAND} ${CARGO_EXECUTABLE} ARGS doc --no-deps
            ${CARGO_ARGS}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${TARGET_OUTPUT}/../doc
            ${CMAKE_BINARY_DIR}/doc_${CARGO_NAME}
    WORKING_DIRECTORY ${CARGO_WORKING_DIRECTORY}
    DEPENDS ${LIB_SOURCES}
    COMMENT "Running: cargo doc ${CARGO_NAME}")
  add_custom_target(${CARGO_NAME}_doc_target DEPENDS ${DOC_FILE})

  if(NOT CARGO_LIB)
    # the cargo "deb" command expects binaries in the release output folder to
    # make it work for debug, copy the debug output to the release output
    #
    # deb are build with --no-buid, this is because if we let cargo deb build it
    # will try to release all cargo packages. Also non related crates
    add_custom_command(
      OUTPUT ${DEB_FILE}
      COMMAND
        LIB_BUILD_TYPE=${LIB_BUILD_TYPE}
        deb_release_dir=${CMAKE_CURRENT_BINARY_DIR}/${LIB_TARGET}/release
        binary_dir=${TARGET_OUTPUT}/${CARGO_NAME};
        ${PROJECT_SOURCE_DIR}/scripts/build/rust_cp_for_cargo_deb.sh
      COMMAND
        ${CARGO_ENV_COMMAND} ${CARGO_EXECUTABLE} ARGS deb --no-strip --no-build
        --deb-version ${APP_VERSION} --output ${DEB_FILE} --target ${LIB_TARGET}
      WORKING_DIRECTORY ${CARGO_WORKING_DIRECTORY}
      DEPENDS ${LIB_SOURCES} ${TARGET_EXECUTABLE}
      COMMENT "Running: cargo deb ${CARGO_NAME}"
      VERBATIM)
    add_custom_target(${CARGO_NAME}_deb_target DEPENDS ${DEB_FILE})
    add_dependencies(deb ${CARGO_NAME}_deb_target)
  endif()

  add_custom_command(
    OUTPUT doc_${CARGO_NAME}/${CARGO_NAME}_${CMAKE_PROJECT_VERSION}.docs.zip
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS ${CARGO_NAME}_doc_target
    COMMAND
      zip -r -q
      doc_${CARGO_NAME}/${CARGO_NAME}_${CMAKE_PROJECT_VERSION}.docs.zip
      doc_${CARGO_NAME})

  add_custom_target(
    ${CARGO_NAME}_doc_zip_target
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS doc_${CARGO_NAME}/${CARGO_NAME}_${CMAKE_PROJECT_VERSION}.docs.zip
    COMMAND
      zip -r -q
      doc_${CARGO_NAME}/${CARGO_NAME}_${CMAKE_PROJECT_VERSION}.docs.zip
      doc_${CARGO_NAME})

  add_dependencies(doxygen ${CARGO_NAME}_doc_target)
  add_dependencies(doxygen_zip ${CARGO_NAME}_doc_zip_target)
endfunction()

# This function exports build configuration of specified cmake targets. Its used
# for 2 Rust related tasks:

# * The rust compiler uses this data so it knows how to build against these
#   native libraries.
# * Rust has a bindings generators that produces binding files based on this
#   exported data see rust's project: uic_bindgen.
#
# These configuration files are .json files that end up in the CMAKE_BINARY_DIR.
# e.g. `my_configuration_bindings.json` uic_bindgen requires these files to be
# there in order to function.
#
# These configuration files are .json files that end up in the CMAKE_BINARY_DIR.
# e.g. `my_configuration_bindings.json` uic_bindgen requires these files to be
# there in order to function.
#
# usage: `export_target_json(my_component)` - basic declaration. my_component
# needs to be an valid cmake target.
#
# `export_target_json(my_component ALLOWLIST "my_component$" BLOCKLIST
# "my_block.*")` - configure the rust binding generator to create bindings for
# vars, functions and types that end of name "my_components". Dont create
# bindings for vars, functions, types that start with "my_block" ALLOWLIST and
# BLOCKLIST both accept regex in order to maniplutate of which items to create
# bindings for.
#
# `export_target_json(my_component EXT_LIB my_depedent_component)` - with
# EXT_LIB you can define adittional dependencies that are needed in order to
# help the rust compiler to resolve dependant libraries. On default will
# `export_target_json` already export all dependencies of the specified target.
#
# `export_target_json(my_component CUSTOM)` - flagging this method with CUSTOM
# generates a configuration file of a non existing target. this can be useful in
# very specific scnario's. be aware that cmake cannot reslove any link libraries
# in this mode.
#
# possible arguments:
# cmake-format: off
# +-----------+--------------------------------------------------------------------------+----------+
# | Argument  |                             Description                                  | optional |
# +-----------+--------------------------------------------------------------------------+----------+
# | 1st arg   | target to export configuration json for                                  |          |
# | ALLOWLIST | allowing generation of bindings that match the specified regex string    | x        |
# | BLOCKLIST | disallowing generation of bindings that match the specified regex string | x        |
# | CUSTOM    | treat target as a custom target                                          | x        |
# | EXT_LIB   | Additional link libraries to resolve rust targets                        | x        |
# +-----------+--------------------------------------------------------------------------+----------+
# cmake-format: on
#
function(export_target_json)
  list(POP_FRONT ARGV TARGET)
  cmake_parse_arguments(PARSED_ARGS "CUSTOM" "" "EXT_LIB" ${ARGV})

  set(json "{\"target_source_dir\": \"\"}")
  string(JSON json SET ${json} "name" "\"${TARGET}\"")

  if(NOT PARSED_ARGS_CUSTOM)
    string(JSON json SET ${json} "target_source_dir"
           "\"$<TARGET_PROPERTY:${TARGET},SOURCE_DIR>\"")

    get_target_property(target_includes ${TARGET} INCLUDE_DIRECTORIES)
    get_target_property(target_link_libraries ${TARGET} LINK_LIBRARIES)
    list(APPEND target_link_libraries ${TARGET})
  endif()

  list(APPEND target_link_libraries ${PARSED_ARGS_EXT_LIB})
  set(json_link_libs "[]")
  foreach(ll ${target_link_libraries})
    if(TARGET ${ll})
      get_target_property(target_type ${ll} TYPE)
      if(target_type MATCHES "UTILITY" OR target_type MATCHES
                                          "INTERFACE_LIBRARY")
        get_target_property(dep_target_includes ${ll}
                            INTERFACE_INCLUDE_DIRECTORIES)
        if(dep_target_includes)
          list(APPEND target_includes ${dep_target_includes})
        endif()
        continue()
      elseif(target_type MATCHES "STATIC_LIBRARY")
        set(typ "static")
      elseif(target_type MATCHES "SHARED_LIBRARY")
        set(typ "dylib")
      endif()

      get_target_property(dep_target_includes ${ll} INCLUDE_DIRECTORIES)
      list(APPEND target_includes ${dep_target_includes})

      set(json_link_item
          "{ \"name\": \"${ll}\",\"typ\": \"${typ}\",\"search_path\": \"$<TARGET_FILE_DIR:${ll}>\"}"
      )
      string(JSON json_link_libs SET ${json_link_libs} 999 "${json_link_item}")
    endif()
  endforeach()
  string(JSON json SET ${json} "link_libraries" ${json_link_libs})

  set(json_include_dirs "[]")
  list(REMOVE_DUPLICATES target_includes)
  foreach(inc ${target_includes})
    string(JSON json_include_dirs SET ${json_include_dirs} 999 "\"${inc}\"")
  endforeach()
  string(JSON json SET ${json} "include_dirs" ${json_include_dirs})

  file(
    GENERATE
    OUTPUT ${uic_dir}/${TARGET}_bindgen.json
    CONTENT ${json})
endfunction()
