if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # "--allow-multiple-definition" can cause some weird symptons as we leave it
  # up to the linker which symbol gets merged into the test binary.
  add_link_options("-Wl,--unresolved-symbols=ignore-in-object-files"
                   "-Wl,--allow-multiple-definition")
endif()

add_definitions(
  -DSILABS_AF_API -DSILABS_EMBER_CONFIGURATION_DEFAULTS_H
  -DZCL_DIRECTION_SERVER_TO_CLIENT=1 -DZCL_NULL_CLUSTER_ID=0xFFFF -DUC_BUILD=1
  -DPLATFORM_HEADER="platform-header.h")

# ##############################################################################
# Setup Mocks
# ##############################################################################

set(GECKO_SDK_MOCK_HEADERS
    mocks/test_zigbee_host_callbacks.h
    mocks/app/framework/include/af.h
    mocks/app/framework/util/af-main.h
    mocks/app/framework/security/af-security.h
    mocks/app/framework/service-function/sl_service_function.h
    mocks/app/util/ezsp/ezsp.h
    mocks/app/util/zigbee-framework/zigbee-device-common.h
    mocks/app/framework/plugin/network-creator/network-creator.h
    mocks/app/framework/plugin/network-creator-security/network-creator-security.h
    mocks/app/framework/plugin/ota-storage-common/ota-storage.h
    mocks/app/framework/plugin/address-table/address-table.h
    mocks/stack/include/zigbee-security-manager.h)

# Use simplified headers of GSDK functionality
add_mock(geckosdk_mock ${GECKO_SDK_MOCK_HEADERS})
target_include_directories(
  geckosdk_mock
  PUBLIC ${ZIGBEE_HOST_STACK_INCLUDES}
         .
         ../include
         ../src
         ../gen/autogen
         ../gen/config
         ${GeckoSDK_ROOT_DIR}/protocol/zigbee
         ${GeckoSDK_ROOT_DIR}/platform/common/inc
         ${GeckoSDK_ROOT_DIR}/protocol/zigbee/app/framework/common/config
         ${GeckoSDK_ROOT_DIR}/protocol/zigbee/app/framework/plugin/debug-print
         ${GeckoSDK_ROOT_DIR}/protocol/zigbee/stack/include/)

add_dependencies(geckosdk_mock zigbee_host)

# ##############################################################################
# Unit Test Suites
# ##############################################################################

# Network Init Suite
target_add_unittest(
  zigbee_host
  NAME
  zigbee_host_trust_center_init_test
  SOURCES
  zigbee_host_trust_center_init_test.c
  ../src/zigbee_host_trust_center.c
  DEPENDS
  geckosdk_mock)

# Device add Suite
target_add_unittest(
  zigbee_host
  NAME
  zigbee_host_trust_center_add_device_test
  SOURCES
  zigbee_host_trust_center_add_device_test.c
  ../src/zigbee_host_trust_center.c
  DEPENDS
  geckosdk_mock)

# Device Discovery Suite
target_add_unittest(
  zigbee_host
  NAME
  zigbee_host_discovery_test
  SOURCES
  zigbee_host_discovery_test.c
  ../src/zigbee_host_discovery.c
  DEPENDS
  geckosdk_mock)

# Device Remove Suite
target_add_unittest(
  zigbee_host
  NAME
  zigbee_host_trust_center_remove_device_test
  SOURCES
  zigbee_host_trust_center_remove_device_test.c
  ../src/zigbee_host_trust_center.c
  DEPENDS
  geckosdk_mock)
target_include_directories(zigbee_host_trust_center_remove_device_test
                           PRIVATE ../src)

# ZCL Frame Suite
target_add_unittest(
  zigbee_host
  NAME
  zigbee_host_zcl_frame_test
  SOURCES
  zigbee_host_zcl_frame_test.c
  ../src/zigbee_host_zcl_frame.c
  DEPENDS
  geckosdk_mock)

# Cluster Command Parse Suite
target_add_unittest(
  zigbee_host
  NAME
  zigbee_host_cluster_command_parse_test
  SOURCES
  zigbee_host_cluster_command_parse_test.c
  ../src/zigbee_host_cluster_command_parse.c
  DEPENDS
  geckosdk_mock)

target_add_unittest(
  zigbee_host
  NAME
  zigbee_host_attribute_mgmt_test
  SOURCES
  zigbee_host_attribute_mgmt_test.c
  ../src/zigbee_host_attribute_mgmt.c
  ../src/zigbee_host_util.c
  DEPENDS
  geckosdk_mock)

target_add_unittest(
  zigbee_host
  NAME
  zigbee_host_address_table_test
  SOURCES
  zigbee_host_address_table_test.c
  ../src/zigbee_host_address_table.c
  DEPENDS
  geckosdk_mock)

target_add_unittest(
  zigbee_host
  NAME
  zigbee_host_ota_server_test
  SOURCES
  zigbee_host_ota_server_test.c
  ../src/zigbee_host_ota_server.c
  DEPENDS
  geckosdk_mock)

target_add_unittest(
  zigbee_host
  NAME
  zigbee_host_poll_checkin_test
  SOURCES
  zigbee_host_poll_checkin_test.c
  ../src/zigbee_host_poll_checkin.c
  DEPENDS
  geckosdk_mock)
target_compile_definitions(
  zigbee_host_poll_checkin_test
  PRIVATE SL_ZIGBEE_AF_PLUGIN_POLL_CONTROL_CLIENT_DEFAULT_FAST_POLL_TIMEOUT=32)

