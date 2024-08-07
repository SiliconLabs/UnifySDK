include(FetchContent)
find_package(GeckoSDK 2024.6.0 REQUIRED)
find_package(CPCD 4.5.0 REQUIRED)

set(FETCHCONTENT_QUIET False)
set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME uic-otbr)

FetchContent_Declare(
  ot-br-posix
  URL ${GeckoSDK_ROOT_DIR}/util/third_party/ot-br-posix
  SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/ot-br-posix"
)

FetchContent_GetProperties(ot-br-posix)
FetchContent_Populate(ot-br-posix)

FetchContent_Declare(
  openthread
  URL ${GeckoSDK_ROOT_DIR}/util/third_party/openthread
  SOURCE_DIR "${ot-br-posix_SOURCE_DIR}/third_party/openthread/repo"
)

FetchContent_Populate(openthread)

set(SILABS_POSIX_CONFIG_PATH "${GeckoSDK_ROOT_DIR}/protocol/openthread/platform-abstraction/posix")

# Set CPCD_SOURCE_DIR to source path exported from FetchContent in FindCPCD
set(CPCD_SOURCE_DIR ${cpcd_SOURCE_DIR} CACHE STRING "Root path to cpc daemon source files")

# Set project options for runtime configurable spinel
set(OT_PLATFORM_CONFIG "${SILABS_POSIX_CONFIG_PATH}/openthread-core-silabs-posix-config.h" CACHE STRING "SiLabs posix config header")
option(OT_MULTIPAN_RCP "Enable multipan RCP" ON)
option(OT_POSIX_RCP_HDLC_BUS "Enable HDLC bus interface" ON)
option(OT_POSIX_RCP_SPI_BUS "Enable SPI bus interface" ON)
option(OT_POSIX_RCP_VENDOR_BUS "Enable vendor bus interface" ON)
set(OT_POSIX_CONFIG_RCP_VENDOR_INTERFACE "${SILABS_POSIX_CONFIG_PATH}/cpc_interface.cpp" CACHE STRING "Name of vendor interface source file")
set(OT_POSIX_CONFIG_RCP_VENDOR_DEPS_PACKAGE "${SILABS_POSIX_CONFIG_PATH}/posix_vendor_rcp.cmake" CACHE STRING "Name of vendor platform interface module file")

# Include rules for vendor API/CLI extension
set(OT_CLI_VENDOR_EXTENSION "${SILABS_POSIX_CONFIG_PATH}/posix_vendor_cli.cmake" CACHE STRING "Name of cli extension cmake file")

# Compilation settings defined
set(OT_DIAGNOSTIC ON CACHE BOOL "enable diagnostic support")
set(OT_PACKAGE_NAME "SL-OPENTHREAD" CACHE STRING "OpenThread Package Name")
set(OT_FULL_LOGS OFF CACHE BOOL "Enable debug level logging" FORCE)

if(NOT BUILD_TESTING)
  set(ENABLE_CJSON_TEST OFF CACHE BOOL "ENABLE CJSON tests")
  set(ENABLE_TESTING OFF CACHE BOOL "Build mbed TLS tests.")
endif()

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/options.cmake")

add_subdirectory(${ot-br-posix_SOURCE_DIR} ${ot-br-posix_BINARY_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/prerm.in
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/prerm.in)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/postinst.in
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/postinst.in)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/services/uic-otbr.in
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/services/uic-otbr.in)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/postrm.in
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/postrm)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/services/uic-otbr.service.in
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/services/uic-otbr.service)

# Second stage scriptlet configuration - evaluate generator expressions
file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/debconf/prerm
              INPUT  ${CMAKE_CURRENT_BINARY_DIR}/debconf/prerm.in)

file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/debconf/postinst
              INPUT  ${CMAKE_CURRENT_BINARY_DIR}/debconf/postinst.in)

file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/debconf/services/uic-otbr
              INPUT  ${CMAKE_CURRENT_BINARY_DIR}/debconf/services/uic-otbr.in)


# Remove otbr compilation for target setup
# Update otbr-agent config with spinel url
execute_process(
  COMMAND sed -i "/otbr/d" ${ot-br-posix_SOURCE_DIR}/script/setup
  COMMAND sed -i "s/set -euxo pipefail/set -euo pipefail/g" ${ot-br-posix_SOURCE_DIR}/script/_initrc
  COMMAND_ERROR_IS_FATAL ANY
)

install(
  DIRECTORY ${ot-br-posix_SOURCE_DIR}/script
  COMPONENT uic-otbr
  DESTINATION share/otbr
  FILES_MATCHING
  PATTERN *
  PERMISSIONS
    OWNER_READ
    OWNER_WRITE
    OWNER_EXECUTE
    GROUP_EXECUTE
    GROUP_READ
    WORLD_READ
    WORLD_EXECUTE
)

install(
  DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/debconf/services/otbr-agent.service.d
  COMPONENT uic-otbr
  DESTINATION /etc/systemd/system
  FILES_MATCHING
  PATTERN *
  PERMISSIONS
    OWNER_READ
    OWNER_WRITE
    OWNER_EXECUTE
    GROUP_EXECUTE
    GROUP_READ
    WORLD_READ
    WORLD_EXECUTE
)

install(
  DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/debconf/services/otbr-web.service.d
  COMPONENT uic-otbr
  DESTINATION /etc/systemd/system
  FILES_MATCHING
  PATTERN *
  PERMISSIONS
    OWNER_READ
    OWNER_WRITE
    OWNER_EXECUTE
    GROUP_EXECUTE
    GROUP_READ
    WORLD_READ
    WORLD_EXECUTE
)

install(
  DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/debconf/services/otbr-nat44.service.d
  COMPONENT uic-otbr
  DESTINATION /etc/systemd/system
  FILES_MATCHING
  PATTERN *
  PERMISSIONS
    OWNER_READ
    OWNER_WRITE
    OWNER_EXECUTE
    GROUP_EXECUTE
    GROUP_READ
    WORLD_READ
    WORLD_EXECUTE
)

install(
  DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/debconf/services/otbr-firewall.service.d
  COMPONENT uic-otbr
  DESTINATION /etc/systemd/system
  FILES_MATCHING
  PATTERN *
  PERMISSIONS
    OWNER_READ
    OWNER_WRITE
    OWNER_EXECUTE
    GROUP_EXECUTE
    GROUP_READ
    WORLD_READ
    WORLD_EXECUTE
)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/debconf/services/uic-otbr.service
  COMPONENT uic-otbr
  DESTINATION ${OTBR_SYSTEMD_UNIT_DIR}
)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/debconf/services/uic-otbr
  COMPONENT uic-otbr
  DESTINATION /etc/default
)

add_component_to_uic(
  uic-otbr # Package Name
  "OpenThread Border Router"
  "${CMAKE_PROJECT_NAME}-otbr" # Package Debian filename
  "iptables, ipset, uic-cpcd"
  ""
  "${CMAKE_CURRENT_BINARY_DIR}/debconf/postinst;\
   ${CMAKE_CURRENT_BINARY_DIR}/debconf/prerm;\
   ${CMAKE_CURRENT_BINARY_DIR}/debconf/postrm;"
)
