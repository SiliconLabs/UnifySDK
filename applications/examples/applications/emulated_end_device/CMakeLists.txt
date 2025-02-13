add_subdirectory(components)

project(eed)
cmake_minimum_required(VERSION 3.19)

# Example Protocol Controller executable
add_executable(eed main.c)
target_link_libraries(eed
  PUBLIC
    eed_application_monitoring
    eed_config
    eed_datastore
    eed_attribute_store
    eed_stdin
    eed_node_state
    eed_attribute_mapper
    eed_group_dispatch
)

target_compile_definitions(
  eed PRIVATE CMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}"
              CMAKE_PROJECT_VERSION="${CMAKE_PROJECT_VERSION}")


install(TARGETS eed RUNTIME DESTINATION "bin" COMPONENT uic-eed)

install(
  DIRECTORY components/eed_attribute_mapper/rules
  COMPONENT uic-eed
  DESTINATION share/uic
  FILES_MATCHING
  PATTERN *.uam)

if("${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr")
  install(
    DIRECTORY
    DESTINATION /var/lib/uic
    COMPONENT uic-eed)
  install(
    FILES debconf/uic-eed.service
    DESTINATION /lib/systemd/system/
    COMPONENT uic-eed)
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

add_component_to_uic(
  uic-eed # Package Name
  "Universal IOT controller - Emulated End Device"
  "uic-eed" # Package Debian filename
  "libunify" # Package Depends on
  ""
  "${CMAKE_BINARY_DIR}/applications/examples/applications/emulated_end_device/debconf/config;\
    ${CMAKE_BINARY_DIR}/applications/examples/applications/emulated_end_device/debconf/templates;\
    ${CMAKE_BINARY_DIR}/applications/examples/applications/emulated_end_device/debconf/conffiles;\
    ${CMAKE_BINARY_DIR}/applications/examples/applications/emulated_end_device/debconf/postinst;\
    ${CMAKE_BINARY_DIR}/applications/examples/applications/emulated_end_device/debconf/prerm")