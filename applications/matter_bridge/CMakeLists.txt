find_package(Matter REQUIRED)
find_path(nlohmann_json_include nlohmann/json.hpp REQUIRED)

run_zap(zap/gen-templates.json)

set(GENERATED_CLUSTER_LIST
  "${CMAKE_CURRENT_SOURCE_DIR}/src/device_type_mapper/matter_device_types_clusters_list.inc")

set(MATTER_BRIDGE_SRC
  src/matter_bridge_config.c
  src/matter_node_state_monitor/matter_node_state_monitor.cpp
  src/matter_node_state_monitor/matter_endpoint_builder.cpp
  src/matter_node_state_monitor/matter_cluster_interactor.cpp
  src/device_type_mapper/matter_device_translator.cpp
  src/device_type_mapper/matter_device_types_clusters_list_updated.inc
  src/cluster_translator/on_off_command_translator.cpp
  src/cluster_translator/on_off_attribute_translator.cpp
  src/cluster_translator/bridged_device_basic_info_attribute_translator.cpp
  src/cluster_translator/identify_command_translator.cpp
  src/cluster_translator/identify_attribute_translator.cpp
  src/cluster_translator/group_translator.cpp
  src/cluster_translator/group_command_translator.cpp
  src/cluster_translator/level_attribute_translator.cpp
  src/cluster_translator/level_command_translator.cpp
  src/matter_bridge_main.cpp
  src/demo_uic_cli.cpp
  src/attribute_state_cache.cpp
  src/matter_data_storage.cpp
  ${ZAP_CURRENT_OUTPUT_DIR}/matter_device_mapper.inc)

add_executable(unify_matter_bridge ${MATTER_BRIDGE_SRC})
target_link_libraries(
  unify_matter_bridge
  PRIVATE Matter::bridge-common
  uic_log
  uic_config
  uic_main
  uic_datastore
  uic_mqtt
  uic_dotdot_mqtt
  unify_node_state_monitor
  pthread)
target_include_directories(
  unify_matter_bridge
  PRIVATE include src src/matter_node_state_monitor src/cluster_translator
  src/device_type_mapper
  ${ZAP_CURRENT_OUTPUT_DIR}
  ${nlohmann_json_include})

# execute matter_device mapper python script
add_custom_command(
  OUTPUT ${GENERATED_CLUSTER_LIST}
  COMMAND ${Python3_EXECUTABLE}
  ${CMAKE_CURRENT_SOURCE_DIR}/scripts/matter_device_type_parse.py -input ${PROJECT_SOURCE_DIR}/components/uic_dotdot/dotdot-xml/CHIPDevice.xml -output ${GENERATED_CLUSTER_LIST})

# add_dependencies(unify_matter_bridge GENERATED_CLUSTER_LIST_dummy)

# Creating the matter-bridge service
if("${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr")
  install(
    FILES debconf/uic-matter-bridge.service
    DESTINATION /lib/systemd/system/
    COMPONENT uic-matter-bridge)
endif()

install(TARGETS unify_matter_bridge RUNTIME DESTINATION "bin"
  COMPONENT uic-matter-bridge)

install(
  FILES ${MATTER_LIBRARY}
  TYPE LIB
  COMPONENT uic-matter-bridge)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/prerm.in
  ${CMAKE_CURRENT_BINARY_DIR}/debconf/prerm)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/postinst.in
  ${CMAKE_CURRENT_BINARY_DIR}/debconf/postinst)

add_component_to_uic(
  uic-matter-bridge
  "Unified IoT Controller - Matter Bridge"
  "${CMAKE_PROJECT_NAME}-matter-bridge"
  "libuic, libglib2.0-dev, libavahi-client-dev"
  "${CMAKE_BINARY_DIR}/applications/matter_bridge/debconf/postinst;\
   ${CMAKE_BINARY_DIR}/applications/matter_bridge/debconf/prerm;")

# Chip tool install
install(
  FILES ${MATTER_BINARY_PATH}/chip-tool
  DESTINATION "bin"
  COMPONENT matter-chip-tool
  PERMISSIONS
  OWNER_READ
  OWNER_WRITE
  OWNER_EXECUTE
  GROUP_EXECUTE
  GROUP_READ
  WORLD_READ
  WORLD_EXECUTE)

add_component_to_uic(matter-chip-tool "Matter Chip Tool"
  "${CMAKE_PROJECT_NAME}-matter-chip-tool" "" "")

add_subdirectory(test)
