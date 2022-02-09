# Libs2 is designed to be used included as a subproject, but this part allows to
# build libs2 standalone, as is done for C51 in SDK670.
if("${PROJECT_NAME}" STREQUAL "")
  cmake_minimum_required(VERSION 2.8)
  project(libs2 C ASM)

  set(CMAKE_BUILD_TYPE debug)
  add_definitions(-Dx86)

  if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")
    # For gov/cobertura: do not use .c.o on the object files, only .o
    set(CMAKE_C_OUTPUT_EXTENSION_REPLACE 1)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
  endif()
  if(CMAKE_C_COMPILER_ID MATCHES "Clang")
    set(CMAKE_C_FLAGS
        "${CMAKE_C_FLAGS} -Wno-self-assign -Wno-error=unused-command-line-argument"
    )
  endif()

  find_path(
    ZWAVE_INCLUDE_DIR ZW_classcmd.h
    HINTS ENV ZWLIBROOT ${PROJECT_SOURCE_DIR}/..
          ${PROJECT_SOURCE_DIR}/../../ZWave/API
    PATH_SUFFIXES include)
  enable_testing()
endif("${PROJECT_NAME}" STREQUAL "")

include(CMakeFunctions.cmake)

if(WIN32 AND NOT CMAKE_BUILD_TYPE MATCHES Test)
  add_definitions(-DDllExport=__declspec\(dllexport\))
else()
  add_definitions(-DDllExport=extern)
endif()

cross_system_dir("include" "" CROSS_SYSTEM_INCLUDE)
# Setting -DOPTIONAL_SYSTEM_INCLUDE=<optional_include_path> allows to specify
# addional include path(s) when building libs2.
include_directories(include ${ZWAVE_INCLUDE_DIR} ${CROSS_SYSTEM_INCLUDE}
                    ${OPTIONAL_SYSTEM_INCLUDE})

if("${CMAKE_PROJECT_NAME}" STREQUAL "SDK")
  set(SKIP_TESTING true)
endif()

set(CMAKE_C_FLAGS_DEBUG "-g3")
if(NOT DEFINED SKIP_TESTING OR CMAKE_BUILD_TYPE STREQUAL Test)
  enable_testing()
  add_subdirectory(TestFramework)
  add_subdirectory(test)
  message(STATUS "Adding unit test for libs2")
else()
  # Use the SKIP_TESTING variable to prevent warning when running cmake
  set(ignoreMe "${SKIP_TESTING}")
  set(DISABLE_MOCK true)
endif()

add_subdirectory(crypto)
add_subdirectory(protocol)
add_subdirectory(inclusion)
add_subdirectory(doc)
if(NOT DEFINED DISABLE_MOCK)
  add_subdirectory(include/mock)
  add_subdirectory(inclusion/mock)
  add_subdirectory(crypto/mock)
endif()

set(s2_slave_source protocol/S2.c inclusion/s2_inclusion.c)
if(NOT "${CMAKE_PROJECT_NAME}" STREQUAL "SDK")
  add_library(s2_slave ${s2_slave_source})
  target_link_libraries(s2_slave s2crypto aes)

  add_library(s2_controller inclusion/s2_inclusion.c protocol/S2.c)
  set_target_properties(s2_controller PROPERTIES COMPILE_DEFINITIONS
                                                 "ZW_CONTROLLER")
  target_link_libraries(s2_controller s2crypto aes)
  target_include_directories(s2_controller
                             PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
else(NOT "${CMAKE_PROJECT_NAME}" STREQUAL "SDK")
  add_library(s2_slave OBJECT ${s2_slave_source})
  target_include_directories(s2_slave PRIVATE crypto/curve25519/generic)
endif(NOT "${CMAKE_PROJECT_NAME}" STREQUAL "SDK")

target_include_directories(s2_slave
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
