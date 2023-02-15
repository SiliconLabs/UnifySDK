project(unify)
cmake_minimum_required(VERSION 3.19)

set(DEFAULT_COMMON_PATH ${PROJECT_SOURCE_DIR}/..)
if(EXISTS ${COMMON_LOCATION})
  set(COMMON_PATH ${COMMON_LOCATION})
elseif(EXISTS ${DEFAULT_COMMON_PATH})
  set(COMMON_PATH ${DEFAULT_COMMON_PATH})
else()
  message(FATAL_ERROR "Did not find common: ${COMMON_LOCATION}")
endif()
message(STATUS "Found common: ${COMMON_PATH}")

set(CMAKE_MODULE_PATH "${COMMON_PATH}/cmake/modules" ${CMAKE_MODULE_PATH})
foreach(path ${CMAKE_MODULE_PATH})
  cmake_path(SET p NORMALIZE "${path}")
  if(NOT EXISTS ${p})
    message(STATUS "Warning: Did not find directory ${p}")
  endif()
endforeach()

set(CMAKE_INCLUDE_PATH "${COMMON_PATH}/cmake/include" ${CMAKE_INCLUDE_PATH})
foreach(path ${CMAKE_INCLUDE_PATH})
  cmake_path(SET p NORMALIZE "${path}")
  if(NOT EXISTS ${p})
    message(STATUS "Warning: Did not find directory ${p}")
  endif()
endforeach()

enable_language(Rust)

set(paths
  ${COMMON_PATH}/cmake/include/target_interface_libraries.cmake
  ${COMMON_PATH}/cmake/include/uic_helper.cmake
  ${COMMON_PATH}/cmake/include/compatible_platform.cmake
  ${COMMON_PATH}/cmake/include/compiler_options.cmake
  ${COMMON_PATH}/cmake/include/version.cmake
  ${COMMON_PATH}/cmake/include/doxygen.cmake
  ${COMMON_PATH}/cmake/modules/CMakeCargo.cmake
  ${COMMON_PATH}/cmake/include/package-helper.cmake
  ${COMMON_PATH}/cmake/include/zap.cmake
  ${COMMON_PATH}/cmake/include/version_file.cmake
  ${COMMON_PATH}/cmake/include/unittest.cmake
  ${COMMON_PATH}/cmake/include/bundle_static_library.cmake
)

foreach(path ${paths})
  cmake_path(SET v NORMALIZE "${CMAKE_CURRENT_LIST_DIR}")
  cmake_path(APPEND v ${path})
  cmake_path(SET p NORMALIZE ${v})
  if(NOT EXISTS ${p})
    message(STATUS "Warning: Did not find file ${p}")
  else()
    include(${p})
    message(STATUS "Included file ${p}")
  endif()
endforeach()

# Global includes
set(paths
  ${CMAKE_CURRENT_BINARY_DIR}/include
  ${CMAKE_CURRENT_LIST_DIR}/../include
)
foreach(path ${paths})
  cmake_path(SET v NORMALIZE "${CMAKE_CURRENT_LIST_DIR}")
  cmake_path(APPEND v ${path})
  cmake_path(SET p NORMALIZE ${v})
  if(NOT EXISTS ${p})
    message(STATUS "Warning: Did not find directory ${p}")
  else()
    include_directories(include ${p})
    message(STATUS "Included directory ${p}")
  endif()
endforeach()

if(BUILD_TESTING)
  cmake_path(SET p NORMALIZE "${CMAKE_CURRENT_LIST_DIR}")
  cmake_path(APPEND p "testframework")
  if (NOT EXISTS ${p})
    message(STATUS "Warning: Did not find subdirectory ${p}")
  else()
    message(STATUS "Adding subdirectory unify testframework from ${CMAKE_CURRENT_LIST_DIR}")
    add_subdirectory(${p})
  endif()

  cmake_path(APPEND p add_mock.cmake)
  if (NOT EXISTS ${p})
    message(STATUS "Warning: Did not find file ${p}")
  else()
    message(STATUS "Included testframework add_mock.cmake from ${CMAKE_CURRENT_LIST_DIR}")
    include(${p})
  endif()
endif()

cargo_build(NAME unify_build MANIFEST_PATH ${CMAKE_CURRENT_LIST_DIR})
cargo_as_cmake_target(unify_build unify_attribute_poll)

set(paths
  uic_definitions
  uic_attribute_mapper
  uic_attribute_resolver
  uic_attribute_store
  uic_attribute_utils
  unify_attribute_poll
  uic_config
  uic_contiki
  uic_datastore
  uic_dotdot
  unify_dotdot_attribute_store
  uic_dotdot_mqtt
  uic_log
  uic_main
  uic_main_fd
  uic_mqtt
  unify_validator
  uic_multi_invoke
  uic_smartstart_management
  uic_stdin
  uic_ota
  uic_gbl_interface
  unify_node_state_monitor
  uic_rust
  unify_application_monitoring
)
foreach(path ${paths})
  cmake_path(SET v NORMALIZE "${CMAKE_CURRENT_LIST_DIR}")
  cmake_path(APPEND v ${path})
  cmake_path(SET p NORMALIZE ${v})
  if(NOT EXISTS ${p})
    message(STATUS "Warning: Did not find subdirectory ${p}")
  else()
    add_subdirectory(${p})
    message(STATUS "Added subdirectory ${p}")
  endif()
endforeach()

# set dependency to attribute store so the attribute poll mock can succesfully
# be build
set_target_properties(unify_attribute_poll PROPERTIES LINK_LIBRARIES
                                                      uic_attribute_store)

set(unify-components
    unify_attribute_poll
    rust_interface_headers
    uic_attribute_mapper
    uic_attribute_resolver
    uic_attribute_store
    uic_attribute_utils
    uic_config
    uic_contiki
    uic_contiki_platform
    uic_datastore
    uic_definitions
    uic_dotdot
    unify_dotdot_attribute_store
    uic_dotdot_mqtt
    uic_gbl_interface
    uic_log
    uic_main
    uic_main_fd
    uic_mqtt
    uic_multi_invoke
    uic_ota
    uic_smartstart_management
    uic_stdin
    unify_node_state_monitor
    unify_validator
    unify_application_monitoring)

add_library(unify SHARED)
add_library(unify_static_non_bundle STATIC)

foreach(component ${unify-components})
  if(TARGET ${component})
    set_property(TARGET ${component} PROPERTY POSITION_INDEPENDENT_CODE 1)
  endif()
endforeach()

target_link_libraries(unify PRIVATE ${unify-components})
# this is a workaround in order to expose the include directories of the
# components while keeping them included as PRIVATE. (required in order for the
# components not to be packaged next to libunify).
set_target_properties(
  unify PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
                   $<TARGET_PROPERTY:unify,INCLUDE_DIRECTORIES>)

# Create Static library for libunify. CMake doesn't support bundling static
# libraries, so instead we create lib called unify_static_non_bundle, that
# doesn't include the other static libs it depends on (rust libraries), after
# that we use bundle_static_library to create unify_static lib, that is
# bundled/includes all other static libs it depends on.
target_link_libraries(unify_static_non_bundle PRIVATE ${unify-components})
bundle_static_library(unify_static_non_bundle unify_static)

target_include_directories(
  unify
  INTERFACE $<INSTALL_INTERFACE:include/uic/sys>
            $<INSTALL_INTERFACE:include/uic/lib>
            $<INSTALL_INTERFACE:include/uic>)

install(
  TARGETS unify
  DESTINATION lib
  COMPONENT libunify)

# Because unify_static is
install(
  FILES $<TARGET_PROPERTY:unify_static,IMPORTED_LOCATION>
  DESTINATION lib
  COMPONENT libunify-dev)

install(
  DIRECTORY "${COMMON_PATH}/include/"
  DESTINATION include/uic
  COMPONENT libunify-dev)

install(
  TARGETS unify
  DESTINATION lib
  COMPONENT libunify-dev
  EXPORT unify-targets)

install(
  EXPORT unify-targets
  FILE unify-config.cmake
  DESTINATION lib/cmake)
export(PACKAGE libunify-dev)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/libunify.pc
        ${CMAKE_CURRENT_BINARY_DIR}/libunify_static.pc
  DESTINATION share/pkgconfig
  COMPONENT libunify-dev)

# --> REFACTOR ME: link dependencies flying all over the place
if(APPLE)
  set(POSTFIX "-mt")
endif()
# ideally this is deducted from the unify target
list(
  APPEND
  UNIFY_EXTERNAL_LIBS
  "boost_log${POSTFIX}"
  "boost_log_setup${POSTFIX}"
  "boost_thread${POSTFIX}"
  "boost_program_options${POSTFIX}"
  "sqlite3"
  "readline"
  "mosquitto"
  "yaml-cpp"
  "pthread"
  "boost_filesystem${POSTFIX}")

# hardcoded required libs for portable runtime environment. this list is not
# complete, and not responsive to changing dependencies that might happen in the
# future.
set(UNIFY_NONE_EXHAUSTIVE_LIST "libboost-program-options-dev (>= 1.67.0),\
libedit2,libsqlite3-0,libmosquitto1,libyaml-cpp0.6,libboost-log-dev (>= 1.67.0)")
# END OF REFACTOR ME SECTION

list(JOIN UNIFY_EXTERNAL_LIBS " -l" PC_LIBS)
set(PC_LIBS "-l${PC_LIBS}")
configure_file(assets/libunify.pc.in libunify.pc @ONLY)
configure_file(assets/libunify_static.pc.in libunify_static.pc @ONLY)

add_component_to_uic(libunify-dev "Unify framework development package"
                     "libunify-dev" "${UNIFY_NONE_EXHAUSTIVE_LIST}" "libunify" "")

add_component_to_uic(libunify "Unify framework package" "libunify"
                     "${UNIFY_NONE_EXHAUSTIVE_LIST}" "libunify" "")

if(BUILD_TESTING)
  target_add_mock(unify_attribute_poll)
endif()

# This shall be at the end of the CMakeLists file
cmake_path(SET v NORMALIZE "${CMAKE_CURRENT_LIST_DIR}")
cmake_path(APPEND v "../cmake/include/package.cmake")
cmake_path(SET p NORMALIZE ${v})
if (NOT EXISTS ${p})
  message(STATUS "Warning: Did not find ${p}")
else()
  include(${p})
endif()
