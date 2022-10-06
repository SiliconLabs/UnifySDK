find_package(PkgConfig REQUIRED)
pkg_check_modules(deps REQUIRED IMPORTED_TARGET glib-2.0)

if(APPLE)
  set(TARGET "x86_64-apple-darwin")
elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
  set(TARGET "x86_64-linux-gnu")
elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "armhf")
  set(TARGET "arm-linux-gnueabihf")
else()
  message(ERROR "Unsupported target")
endif()

# todo find a better place as inside the docker folder ?!
set(MATTER_ROOT "${CMAKE_SOURCE_DIR}/docker/uic-resources/matter")
set(MATTER_BINARY_PATH
  "${CMAKE_SOURCE_DIR}/docker/uic-resources/matter/bin/${TARGET}")

find_library(MATTER_LIBRARY matter matterd libmatter REQUIRED
  HINTS "${MATTER_ROOT}/lib/${TARGET}")

set(MATTER_INCLUDE_PATH "${MATTER_ROOT}/include/${TARGET}")

if(NOT EXISTS "${MATTER_INCLUDE_PATH}")
  message(FATAL_ERROR "could not find matter include path")
endif()

set(INCLUDES_TXT "${MATTER_ROOT}/include/${TARGET}/includes.txt")

if(NOT EXISTS "${INCLUDES_TXT}")
  message(FATAL_ERROR "could not read includes input file: ${INCLUDES_TXT}")
endif()

add_library(Matter::bridge-common SHARED IMPORTED)
set_target_properties(
  Matter::bridge-common
  PROPERTIES IMPORTED_LOCATION "${MATTER_LIBRARY}"
  INSTALL_RPATH_USE_LINK_PATH TRUE
  IMPORTED_NO_SONAME TRUE)
target_link_libraries(Matter::bridge-common INTERFACE PkgConfig::deps)

target_compile_definitions(
  Matter::bridge-common
  INTERFACE 
    CHIP_ADDRESS_RESOLVE_IMPL_INCLUDE_HEADER=\<lib/address_resolve/AddressResolve_DefaultImpl.h\>
    INET_TCP_END_POINT_IMPL_CONFIG_FILE=\<inet/TCPEndPointImplSockets.h\>
    INET_UDP_END_POINT_IMPL_CONFIG_FILE=\<inet/UDPEndPointImplSockets.h\>

    )

file(READ "${INCLUDES_TXT}" INCLUDE_PATHS)
string(REGEX REPLACE "\n" ";" INCLUDE_PATHS "${INCLUDE_PATHS}")

foreach(P ${INCLUDE_PATHS})
  string(SUBSTRING ${P} 2 -1 INCL)
  list(APPEND MATTER_INCLUDES "${MATTER_INCLUDE_PATH}/${INCL}")
endforeach()

set_target_properties(
  Matter::bridge-common PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
  "${MATTER_INCLUDES}")

add_executable(Matter::chip-utils IMPORTED)
set_target_properties(Matter::chip-utils PROPERTIES IMPORTED_LOCATION
  "${MATTER_LIBRARY}")
set_target_properties(
  Matter::chip-utils PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
  "${MATTER_INCLUDES}")
