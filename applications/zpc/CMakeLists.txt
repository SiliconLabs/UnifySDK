if(NOT ZPC_SOURCE_DIR)
  set(ZPC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif(NOT ZPC_SOURCE_DIR)

project(zpc)
cmake_minimum_required(VERSION 3.19)

set(DEFAULT_COMMON_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../..)
if(EXISTS ${COMMON_LOCATION})
  set(COMMON_PATH ${COMMON_LOCATION})
else()
  set(COMMON_PATH ${DEFAULT_COMMON_PATH})
endif()
if(NOT EXISTS ${COMMON_PATH})
  message(FATAL_ERROR "Path to common does not exist")
else()
  message(STATUS "Found common: ${COMMON_PATH}")
endif()

set(CMAKE_MODULE_PATH ${COMMON_PATH}/cmake/modules ${CMAKE_MODULE_PATH})
set(CMAKE_INCLUDE_PATH ${COMMON_PATH}/cmake/include ${CMAKE_INCLUDE_PATH})
enable_language(Rust)

include(${COMMON_PATH}/cmake/include/target_interface_libraries.cmake)
include(${COMMON_PATH}/cmake/include/uic_helper.cmake)
include(${COMMON_PATH}/cmake/include/compatible_platform.cmake)
include(${COMMON_PATH}/cmake/include/compiler_options.cmake)
include(${COMMON_PATH}/cmake/include/version.cmake)
include(${COMMON_PATH}/cmake/include/doxygen.cmake)
include(${COMMON_PATH}/cmake/modules/CMakeCargo.cmake)
include(${COMMON_PATH}/cmake/include/package-helper.cmake)
include(${COMMON_PATH}/cmake/include/zap.cmake)
include(${COMMON_PATH}/cmake/include/unittest.cmake)
include(${COMMON_PATH}/cmake/include/version_file.cmake)

set(DEFAULT_UNIFY-TESTFRAMEWORK_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/testframework)
if(EXISTS ${UNIFY-TESTFRAMEWORK_LOCATION})
  set(UNIFY-TESTFRAMEWORK_PATH ${UNIFY-TESTFRAMEWORK_LOCATION})
else()
  set(UNIFY-TESTFRAMEWORK_PATH ${DEFAULT_UNIFY-TESTFRAMEWORK_PATH})
endif()
if(NOT EXISTS ${UNIFY-TESTFRAMEWORK_PATH})
  message(FATAL_ERROR "Path to unify testframework does not exist")
else()
  message(STATUS "Found unify testframwork: ${UNIFY-TESTFRAMEWORK_PATH}")
endif()

include(${UNIFY-TESTFRAMEWORK_PATH}/add_mock.cmake)
include(${UNIFY-TESTFRAMEWORK_PATH}/target_add_unittest.cmake)


if(EXISTS ${LIBUNIFY_LOCATION})
  message(STATUS "Found libunify: ${LIBUNIFY_LOCATION}")
  list(APPEND CMAKE_PREFIX_PATH ${LIBUNIFY_LOCATION}/lib/cmake)
  find_package(unify REQUIRED)
else()
# find_package(unify REQUIRED)
endif()

add_subdirectory(components)

# ZPC executable
add_executable(zpc main.c)
target_link_libraries(
  zpc
  PUBLIC dotdot_mapper
         dotdot_mapper_binding_cluster_helper
         network_monitor

         ucl_mqtt
         zcl_cluster_servers
         zpc_application_monitoring
         zpc_attribute_mapper
         zpc_attribute_resolver
         zpc_attribute_store
         zpc_config
         zpc_datastore
         zpc_dotdot_mqtt
         zpc_ncp_update
         zpc_rust
         zpc_stdin
         zpc_utils
         zwave_controller
         zwave_definitions
         zwave_network_management
         zwave_rx
         zwave_transports
         zwave_tx
         zwave_tx_groups
         zwave_tx_scheme_selector

         zwave_api
         zwave_security_validation
         zwave_s2
         zwave_smartstart_management
         zwave_command_handler
         zwave_command_classes)

target_compile_definitions(
  zpc PRIVATE CMAKE_PROJECT_VERSION="${CMAKE_PROJECT_VERSION}")

target_include_directories(zpc PUBLIC components/zwave/zwave_transports)

install(TARGETS zpc RUNTIME DESTINATION "bin" COMPONENT uic-zpc)

install(
  FILES scripts/bash-completion/zpc
  DESTINATION share/bash-completion/completions
  COMPONENT uic-zpc)

install(
  DIRECTORY components/dotdot_mapper/rules
  COMPONENT uic-zpc
  DESTINATION share/uic
  FILES_MATCHING
  PATTERN *.uam)

if("${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr")
  install(
    DIRECTORY
    DESTINATION /var/lib/zpc
    COMPONENT uic-zpc)

  install(
    FILES scripts/systemd/uic-zpc.service
    DESTINATION /lib/systemd/system/
    COMPONENT uic-zpc)

  install(
    FILES scripts/udev/99-leds.rules
    DESTINATION /etc/udev/rules.d/
    COMPONENT uic-zpc)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/config.in
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/config)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/postinst.in
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/postinst)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/postrm.in
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/postrm)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/prerm.in
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/prerm)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/templates.in
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/templates)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/conffiles.in
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/conffiles)

include(scripts/scripts.cmake)
add_component_to_uic(
  uic-zpc # Package Name
  "Universal IOT controller - Z-Wave protocol Controller"
  "uic-zpc" # Package Debian filename
  "libunify" # Package Depends on
  ""
  "${CMAKE_BINARY_DIR}/applications/zpc/debconf/config;\
    ${CMAKE_BINARY_DIR}/applications/zpc/debconf/templates;\
    ${CMAKE_BINARY_DIR}/applications/zpc/debconf/conffiles;\
    ${CMAKE_BINARY_DIR}/applications/zpc/debconf/postinst;\
    ${CMAKE_BINARY_DIR}/applications/zpc/debconf/prerm")
