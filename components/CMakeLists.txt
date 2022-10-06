project(uic)
cmake_minimum_required(VERSION 3.19)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules"
                      ${CMAKE_MODULE_PATH})
set(CMAKE_INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/include"
                       ${CMAKE_INCLUDE_PATH})
enable_language(Rust)

include(../cmake/include/target_interface_libraries.cmake)
include(../cmake/include/uic_helper.cmake)
include(../cmake/include/compatible_platform.cmake)
include(../cmake/include/compiler_options.cmake)
include(../cmake/include/version.cmake)
include(../cmake/include/doxygen.cmake)
include(../cmake/modules/CMakeCargo.cmake)
include(../cmake/include/package-helper.cmake)
include(../cmake/include/zap.cmake)
include(../cmake/include/version_file.cmake)

add_subdirectory(testframework)
add_subdirectory(uic_definitions)
add_subdirectory(uic_attribute_mapper)
add_subdirectory(uic_attribute_resolver)
add_subdirectory(uic_attribute_store)
add_subdirectory(uic_attribute_utils)
add_subdirectory(unify_attribute_poll)
add_subdirectory(uic_config)
add_subdirectory(uic_contiki)
add_subdirectory(uic_datastore)
add_subdirectory(uic_dotdot)
add_subdirectory(uic_dotdot_attribute_store)
add_subdirectory(uic_dotdot_mqtt)
add_subdirectory(uic_log)
add_subdirectory(uic_main)
add_subdirectory(uic_main_fd)
add_subdirectory(uic_mqtt)
add_subdirectory(unify_validator)
add_subdirectory(uic_multi_invoke)
add_subdirectory(uic_smartstart_management)
add_subdirectory(uic_stdin)
add_subdirectory(uic_rust)
add_subdirectory(uic_ota)
add_subdirectory(uic_gbl_interface)
add_subdirectory(unify_node_state_monitor)

set(unify_components
    unify_attribute_poll
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
    uic_dotdot_attribute_store
    uic_dotdot_mqtt
    uic_gbl_interface
    uic_log
    uic_main
    uic_main_fd
    uic_mqtt
    uic_mqtt_client
    uic_mqtt_wrapper
    uic_multi_invoke
    uic_ota
    uic_smartstart_management
    uic_stdin
    unify_node_state_monitor
    unify_validator)

foreach(item ${unify_components})
  get_target_property(target_type ${item} TYPE)
  if(NOT target_type STREQUAL "INTERFACE_LIBRARY")
    list(APPEND target-objects "\$<TARGET_OBJECTS:${item}>")
  endif()
  get_target_property(includes ${item} INTERFACE_INCLUDE_DIRECTORIES)
  list(APPEND target-includes "${includes}")
endforeach()

add_library(unify ${target-objects})
target_link_libraries(unify ${unify_components})

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/libunify.pc
  DESTINATION share/pkgconfig
  COMPONENT libuic-dev)

# This is not a real solution...
if(APPLE)
  set(UNIFY_EXTERNAL_LIBS
      "-lboost_log-mt  -lboost_log_setup-mt -lboost_thread-mt -lboost_program_options-mt -lsqlite3 -lreadline -lmosquitto -lyaml-cpp -lpthread"
  )
else()
  set(UNIFY_EXTERNAL_LIBS
      "-lboost_log  -lboost_log_setup -lboost_thread -lboost_program_options -lsqlite3 -lreadline -lmosquitto -lyaml-cpp -lpthread"
  )
endif()

configure_file(assets/libunify.pc.in libunify.pc @ONLY)

install(
  FILES ${headers}
  DESTINATION uic
  COMPONENT unify-dev)
install(
  FILES $<TARGET_FILE:unify>
  DESTINATION lib
  COMPONENT unify-dev)
export(PACKAGE unify-dev)

