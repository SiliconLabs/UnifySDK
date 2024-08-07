find_package(Ruby)
# The RUBY_EXECUTABLE variable does not exist when the ADD_UNITY2_TEST function
# is called So instaed store the value in cache and use the cached value
set(UNITY2_RUBY_EXECUTABLE
    ${RUBY_EXECUTABLE}
    CACHE INTERNAL "")

set(DIR_OF_TARGET_ADD_UNIT_TEST
    ${CMAKE_CURRENT_LIST_DIR}
    CACHE INTERNAL "DIR_OF_TARGET_ADD_UNIT_TEST")

function(generate_unity_runner test_runner test_file)
  if(EXISTS ${UNIFY-TESTFRAMEWORK_LOCATION})
    set(ZWAVE_UNITY_CONFIG
        ${UNIFY-TESTFRAMEWORK_LOCATION}/unify_cmock_config.yml)
  else()
    set(ZWAVE_UNITY_CONFIG
        ${DIR_OF_TARGET_ADD_UNIT_TEST}/unify_cmock_config.yml)
  endif()
  if(NOT EXISTS ${ZWAVE_UNITY_CONFIG})
    message(
      FATAL_ERROR "Did not find unify_cmock_config.yml at ${ZWAVE_UNITY_CONFIG}"
    )
  endif()
  if(EXISTS ${THS-UNITY_LOCATION})
    set(UNITY_DIR ${THS-UNITY_LOCATION})
    add_custom_command(
      OUTPUT ${TEST_RUNNER}
      DEPENDS ${TEST_FILE}
      COMMAND
        ${UNITY2_RUBY_EXECUTABLE} ${UNITY_DIR}/auto/generate_test_runner.rb
        ${ZWAVE_UNITY_CONFIG} ${TEST_FILE} ${TEST_RUNNER}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  else()
    set(UNITY_DIR "${DIR_OF_TARGET_ADD_UNIT_TEST}/libs/cmock/vendor/unity")
    add_custom_command(
      OUTPUT ${TEST_RUNNER}
      DEPENDS ${TEST_FILE}
      COMMAND
        ${UNITY2_RUBY_EXECUTABLE} ${UNITY_DIR}/auto/generate_test_runner.rb
        ${DIR_OF_TARGET_ADD_UNIT_TEST}/zwave_unity_config.yml ${TEST_FILE}
        ${TEST_RUNNER}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  endif()
endfunction()

# This function creates unity2 test executables. It uses the provided target to setup and import configuration
# to successfully compile a unit-test executable. more specifically, it re-uses the build object files of the provided target
# to relink it with optional mock dependencies into a new executable. It does so using the same include and link directives as the provided target.
# the provided target need to exist and be valid before calling this function.
#
# usage:
# `target_add_unittest(my_component SOURCES test.c)` - creates a cmake target named my_component_test. it will compile test.c and link it against my_component.
#
# `target_add_unittest(my_component SOURCES test.c DEPENDS my_mock)` - creates a cmake target named my_component_test and links target my_mock
#
# `target_add_unittest(my_component SOURCES test.c DEPENDS foo_mock EXCLUDE foo.c)` - leaves out foo.c object when linking the new test target.
#   use when you want to replace it with an mock
#
# possible arguments:
# +-----------+----------------------------------------------------------------------+----------+
# | Argument  |                             Description                              | optional |
# +-----------+----------------------------------------------------------------------+----------+
# | 1st arg   | target to setup an unit-test target for                              |          |
# | NAME      | Overwrite the default name of test executable (default=$Target_test) | x        |
# | SOURCES   | Source files that define unit-tests                                  |          |
# | DEPENDS   | New additional dependencies                                          | x        |
# | EXCLUDE   | Omit certain source files from the imported target                   | x        |
# | DISABLED  | Disable test                                                         | x        |
# +-----------+----------------------------------------------------------------------+----------+
#
function(target_add_unittest)
  # first argument is the target to make unit-test exe of
  list(POP_FRONT ARGV TARGET)

  cmake_parse_arguments(PARSED_ARGS "DISABLED" "NAME" "DEPENDS;SOURCES;EXCLUDE"
                        ${ARGV})
  if(NOT ${PARSED_ARGS_UNPARSED_ARGUMENTS} STREQUAL "")
    message(FATAL_ERROR "could not parse ${PARSED_ARGS_UNPARSED_ARGUMENTS}")
  elseif(NOT ${PARSED_ARGS_KEYWORDS_MISSING_VALUES} STREQUAL "")
    message(
      FATAL_ERROR
        "following keywords are missing values: ${PARSED_ARGS_KEYWORDS_MISSING_VALUES}"
    )
  endif()

  if(NOT DEFINED PARSED_ARGS_NAME)
    set(TEST_TARGET "${TARGET}_test")
  else()
    set(TEST_TARGET "${PARSED_ARGS_NAME}")
  endif()

  # The first file in the source list is the main test file
  list(GET PARSED_ARGS_SOURCES 0 TEST_FILE)
  get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
  set(TEST_RUNNER "${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME}_runner.c")
  generate_unity_runner(${TEST_RUNNER} ${TEST_FILE})

  get_target_property(target_type ${TARGET} TYPE)
  if(target_type STREQUAL "INTERFACE_LIBRARY")
    list(APPEND PARSED_ARGS_DEPENDS ${TARGET})
  else()
    collect_object_files(${TARGET} "${TARGET_OBJS}" "${PARSED_ARGS_EXCLUDE}")
  endif()

  # tests binaries reuse the o files of the actual target and relink them with
  # the correct components nessicary for the test. e.g. mocks/stubs.
  add_executable(${TEST_TARGET} ${PARSED_ARGS_SOURCES} ${TARGET_OBJS}
                                ${TEST_RUNNER})

  target_include_directories(
    ${TEST_TARGET} PRIVATE . $<TARGET_PROPERTY:${TARGET},INCLUDE_DIRECTORIES>)

  target_link_libraries(
    ${TEST_TARGET} PRIVATE unity2 ${PARSED_ARGS_DEPENDS}
                           $<TARGET_PROPERTY:${TARGET},LINK_LIBRARIES>)
  add_test(${TEST_NAME} ${TEST_TARGET})

  if(${PARSED_ARGS_DISABLED})
    set_tests_properties(${TEST_NAME} PROPERTIES DISABLED True)
  endif()
endfunction()

function(target_add_gtest)
  # first argument is the target to make unit-test exe of
  list(POP_FRONT ARGV TARGET)
  cmake_parse_arguments(PARSED_ARGS "DISABLED" "NAME"
                        "DEPENDS;SOURCES;EXCLUDE;INCLUDES" ${ARGV})
  if(NOT ${PARSED_ARGS_UNPARSED_ARGUMENTS} STREQUAL "")
    message(FATAL_ERROR "could not parse ${PARSED_ARGS_UNPARSED_ARGUMENTS}")
  elseif(NOT ${PARSED_ARGS_KEYWORDS_MISSING_VALUES} STREQUAL "")
    message(
      FATAL_ERROR
        "following keywords are missing values: ${PARSED_ARGS_KEYWORDS_MISSING_VALUES}"
    )
  endif()

  if(NOT DEFINED PARSED_ARGS_NAME)
    set(TEST_TARGET "${TARGET}_test")
  else()
    set(TEST_TARGET "${PARSED_ARGS_NAME}")
  endif()

  get_target_property(target_type ${TARGET} TYPE)
  if(target_type STREQUAL "INTERFACE_LIBRARY")
    list(APPEND PARSED_ARGS_DEPENDS ${TARGET})
  else()
    collect_object_files(${TARGET} "${TARGET_OBJS}" "${PARSED_ARGS_EXCLUDE}")
  endif()

  find_package(Threads REQUIRED)
  include(GoogleTest)

  # tests binaries reuse the o files of the actual target and relink them with
  # the correct components nessicary for the test. e.g. mocks/stubs.
  add_executable(${TEST_TARGET} ${PARSED_ARGS_SOURCES} ${TARGET_OBJS})
  target_include_directories(
    ${TEST_TARGET} PRIVATE . $<TARGET_PROPERTY:${TARGET},INCLUDE_DIRECTORIES>
                           ${PARSED_ARGS_INCLUDES})
  target_link_libraries(
    ${TEST_TARGET}
    PRIVATE ${PARSED_ARGS_DEPENDS} $<TARGET_PROPERTY:${TARGET},LINK_LIBRARIES>
            gtest gmock gmock_main pthread)
  gtest_discover_tests(${TEST_TARGET})
endfunction()
