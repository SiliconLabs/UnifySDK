add_definitions(
  -DZ3GATEWAY_TEST
  -DSILABS_AF_API
  -DSILABS_EMBER_CONFIGURATION_DEFAULTS_H
  -DEMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE=250
  -DEMBER_AF_PLUGIN_DEVICE_TABLE_CLUSTER_SIZE=32
  -DZCL_DIRECTION_SERVER_TO_CLIENT=1
  -DZCL_NULL_CLUSTER_ID=0xFFFF
  -DPLATFORM_HEADER="dummy.h"
  -DEMBER_AF_API_AF_HEADER="app/framework/include/af.h"
  -DEMBER_AF_API_AF_MAIN_HEADER="app/framework/util/af-main.h"
  -DEMBER_AF_API_AF_SECURITY_HEADER="app/framework/security/af-security.h"
  -DEMBER_AF_API_EZSP_PROTOCOL="dummy.h"
  -DEMBER_AF_API_EZSP="app/util/ezsp/ezsp.h"
  -DEMBER_AF_API_ADDRESS_TABLE="plugin/address-table/address-table.h"
  -DEMBER_AF_API_DEVICE_TABLE="plugin/device-table/device-table.h"
  -DEMBER_AF_API_DEVICE_TABLE_INTERNAL="plugin/device-table/device-table-internal.h"
  -DEMBER_AF_API_NETWORK_CREATOR="network-creator.h"
  -DEMBER_AF_API_NETWORK_CREATOR_SECURITY="network-creator-security.h")

# ##############################################################################
# Setup Mocks
# ##############################################################################

set(GECKO_SDK_MOCK_HEADERS
    mocks/test_z3gateway_callbacks.h
    mocks/app/framework/include/af.h
    mocks/app/framework/util/af-main.h
    mocks/app/framework/security/af-security.h
    mocks/app/util/ezsp/ezsp.h
    mocks/plugin/network-creator/network-creator.h
    mocks/plugin/network-creator-security/network-creator-security.h
    mocks/plugin/address-table/address-table.h
    mocks/plugin/device-table/device-table.h)

set(GECKO_SDK_MOCK_INCLUDES ../include ../gen ../stack
                            ${Z3GATEWAY_GSDK_INCLUDES})
# Use simplified headers of GSDK functionality
add_mock(geckosdk_mock ${GECKO_SDK_MOCK_HEADERS})
target_include_directories(geckosdk_mock PUBLIC ${GECKO_SDK_MOCK_INCLUDES})

# ##############################################################################
# Unit Test Suites
# ##############################################################################

# Network Init Suite
target_add_unittest(
  z3gateway
  NAME
  z3gateway_trust_center_init_test
  SOURCES
  z3gateway_trust_center_init_test.c
  ../src/z3gateway_trust_center.c
  DEPENDS
  geckosdk_mock)
target_include_directories(z3gateway_trust_center_init_test PRIVATE ../src)

# Device add Suite
target_add_unittest(
  z3gateway
  NAME
  z3gateway_trust_center_add_device_test
  SOURCES
  z3gateway_trust_center_add_device_test.c
  ../src/z3gateway_trust_center.c
  DEPENDS
  geckosdk_mock)
target_include_directories(z3gateway_trust_center_add_device_test
                           PRIVATE ../src)

# Device Endpoint Discovery Suite
target_add_unittest(
  z3gateway
  NAME
  z3gateway_trust_center_endpoint_discovery_test
  SOURCES
  z3gateway_trust_center_endpoint_discovery_test.c
  ../src/z3gateway_trust_center.c
  DEPENDS
  geckosdk_mock)
target_include_directories(z3gateway_trust_center_endpoint_discovery_test
                           PRIVATE ../src)

# Device Remove Suite
target_add_unittest(
  z3gateway
  NAME
  z3gateway_trust_center_remove_device_test
  SOURCES
  z3gateway_trust_center_remove_device_test.c
  ../src/z3gateway_trust_center.c
  DEPENDS
  geckosdk_mock)
target_include_directories(z3gateway_trust_center_remove_device_test
                           PRIVATE ../src)

# Device Table Persist Suite
target_add_unittest(
  z3gateway
  NAME
  z3gateway_device_table_test
  SOURCES
  z3gateway_device_table_test.c
  ../src/z3gateway_device_table.c
  DEPENDS
  geckosdk_mock)
target_include_directories(z3gateway_trust_center_remove_device_test
                           PRIVATE ../src)

# ZCL Frame Suite
target_add_unittest(
  z3gateway
  NAME
  z3gateway_zcl_frame_test
  SOURCES
  z3gateway_zcl_frame_test.c
  ../src/z3gateway_zcl_frame.c
  DEPENDS
  geckosdk_mock)
target_include_directories(z3gateway_zcl_frame_test PRIVATE ../src)

# Cluster Command Parse Suite
target_add_unittest(
  z3gateway
  NAME
  z3gateway_cluster_command_parse_test
  SOURCES
  z3gateway_cluster_command_parse_test.c
  ../src/z3gateway_cluster_command_parse.c
  DEPENDS
  geckosdk_mock)
target_include_directories(z3gateway_cluster_command_parse_test PRIVATE ../src)
