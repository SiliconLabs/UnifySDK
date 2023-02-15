find_package(Ruby)
# The RUBY_EXECUTABLE variable does not exist when the ADD_MOCK function is
# called So instaed store the value in cache and use the cached value
set(CMOCK_RUBY_EXECUTABLE
    ${RUBY_EXECUTABLE}
    CACHE INTERNAL "")

set(DIR_OF_ADD_MOCK_CMAKE
    ${CMAKE_CURRENT_LIST_DIR}
    CACHE INTERNAL "DIR_OF_ADD_MOCK_CMAKE")

# Documentation add_mock(<TARGET_NAME> header1 [header2] ... [headerN]  )
#
# Make a mock library using cmake given a list of header files.
function(ADD_MOCK TARGET)
  set(CMOCK_DIR "${DIR_OF_ADD_MOCK_CMAKE}")

  set(MOCK_HEADERS ${ARGV})
  # Pop the first argument of the list
  list(REMOVE_AT MOCK_HEADERS 0)

  foreach(HEADER ${MOCK_HEADERS})
    get_filename_component(BASE ${HEADER} NAME_WE)
    get_filename_component(DIR ${HEADER} DIRECTORY)

    if(IS_ABSOLUTE ${HEADER})
      list(APPEND MOCK_HEADER_FILES "${HEADER}")
      list(APPEND MOCK_HEADER_DIRS "${DIR}")
    else()
      list(APPEND MOCK_HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${HEADER}")
      list(APPEND MOCK_HEADER_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/${DIR}")
    endif()

    list(APPEND MOCK_SOURCE_FILES
         "${CMAKE_CURRENT_BINARY_DIR}/mocks/${BASE}_mock.c")
  endforeach()

  list(REMOVE_DUPLICATES MOCK_HEADER_DIRS)
  
  if(NOT EXISTS ${UNIFY-TESTFRAMEWORK_LOCATION})
    set(UNIFY-TESTFRAMEWORK_LOCATION ${DIR_OF_ADD_MOCK_CMAKE})
  endif()

  set(ZWAVE_UNITY_CONFIG ${UNIFY-TESTFRAMEWORK_LOCATION}/zwave_cmock_config.yml)

  if(NOT EXISTS ${ZWAVE_UNITY_CONFIG})
    message(
      FATAL_ERROR
        "Did not find zwave_cmock_config.yml at ${ZWAVE_UNITY_CONFIG}")
  endif()

  if((EXISTS ${THS-CMOCK_LOCATION}) AND (EXISTS ${THS-UNITY_LOCATION}))    
    set(UNITY_DIR ${THS-UNITY_LOCATION})
    add_custom_command(
      OUTPUT ${MOCK_SOURCE_FILES}
      DEPENDS ${MOCK_HEADER_FILES}
      COMMAND
        UNITY_DIR=${THS-UNITY_LOCATION} ${CMOCK_RUBY_EXECUTABLE}
        ${THS-CMOCK_LOCATION}/lib/cmock.rb -o${ZWAVE_UNITY_CONFIG}
        ${MOCK_HEADER_FILES})
  else()
    if (NOT EXISTS ${CMOCK_DIR}/libs/cmock/lib/cmock.rb)
      message(FATAL_ERROR "Did not find cmock.rb.")
    endif()
    add_custom_command(
      OUTPUT ${MOCK_SOURCE_FILES}
      DEPENDS ${MOCK_HEADER_FILES}
      COMMAND ${CMOCK_RUBY_EXECUTABLE} ${CMOCK_DIR}/libs/cmock/lib/cmock.rb
              -o${CMOCK_DIR}/zwave_cmock_config.yml ${MOCK_HEADER_FILES})
  endif()

  add_library(${TARGET} ${MOCK_SOURCE_FILES})
  target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/mocks
                                              ${MOCK_HEADER_DIRS})
  target_link_libraries(${TARGET} PUBLIC cmock)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${TARGET} PRIVATE "-fPIC")
  endif()
endfunction()

# Generates a mocking library based of the provided target.
# It will generate mock implemenations for every header in the public include directory of the provided target.
# uses unity/cmock to generate mocks.
function(target_add_mock)
  # first argument is the target to make unit-test exe of.
  list(POP_FRONT ARGV TARGET)
  get_target_property(incl_dirs ${TARGET} INTERFACE_INCLUDE_DIRECTORIES)
  set(headers "")
  foreach(dir ${incl_dirs})
    # strip out any includes that wrapped in BUILD_INTERFACE expressions if present
    string(REGEX
           REPLACE "\\s*(:?\\\$<BUILD_INTERFACE:)?([A-Za-z0-9_/\\\${}-]+)>?"
                   "\\2" dir "${dir}")
    file(GLOB files ${dir}/*.h)
    list(APPEND headers "${files}")
  endforeach()

  # add additional headers
  list(APPEND headers "${ARGV}")

  list(LENGTH headers len)
  if(NOT ${len} EQUAL 0)
    add_mock("${TARGET}_mock" ${headers})
    get_target_property(libs ${TARGET} LINK_LIBRARIES)
    foreach(lib ${libs})
      # filter libs-list so actual cmake targets are left.
      # $<TARGET_EXISTS is not robust enough, filter out files with extension.
      # we know already they are not actual cmake targets.
      string(FIND "${lib}" "." found_index REVERSE)
      if(${found_index} EQUAL -1)
        target_include_directories(
          "${TARGET}_mock"
          PUBLIC
            "$<$<TARGET_EXISTS:${lib}>:$<TARGET_PROPERTY:${lib},INTERFACE_INCLUDE_DIRECTORIES>>"
        )
      endif()
    endforeach()
  else()
    message(WARNING "Nothing to mock for ${TARGET}. no headers found")
    message(WARNING "looked in \"${incl_dirs}\"")
  endif()

endfunction()
