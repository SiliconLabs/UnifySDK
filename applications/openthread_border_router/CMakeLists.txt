include(FetchContent)
find_package(GeckoSDK 4.3.0 REQUIRED)
find_package(CPCD 4.3.0 REQUIRED)

set(FETCHCONTENT_QUIET False)
set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME uic-otbr)

# ot-br-posix unfortunately has a variable set but not used, making our compiler fail because of warnings as errors.
add_compile_options(-Wno-unused-but-set-variable)

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

# Install compile-time configuration constants header file
set(SILABS_POSIX_CONFIG_PATH "${GeckoSDK_ROOT_DIR}/protocol/openthread/platform-abstraction/posix")
file(GLOB POSIX_CONFIG_HEADERS "${SILABS_POSIX_CONFIG_PATH}/openthread-core-silabs-posix-config.h")
file(COPY ${POSIX_CONFIG_HEADERS} DESTINATION ${openthread_SOURCE_DIR}/src/posix/platform)

# Set CPCD_SOURCE_DIR to source path exported from FetchContent in FindCPCD
set(CPCD_SOURCE_DIR ${cpcd_SOURCE_DIR} CACHE STRING "Root path to cpc daemon source files")

# Set project options for CPC as RCP vendor implementation
set(OT_POSIX_CONFIG_RCP_BUS "VENDOR" CACHE STRING "RCP bus type")
set(OT_POSIX_CONFIG_RCP_VENDOR_INTERFACE "${SILABS_POSIX_CONFIG_PATH}/cpc_interface.cpp" CACHE STRING "Name of vendor interface source file")

# Temporarily remove call to cpc FindModule
# and link directly to cpc after add_subdirectory
#set(OT_POSIX_CONFIG_RCP_VENDOR_DEPS_PACKAGE "SilabsRcpDeps" CACHE STRING "Name of vendor interface module file")
#list(APPEND CMAKE_MODULE_PATH "${SILABS_POSIX_CONFIG_PATH}")

# Include rules for vendor API/CLI extension
set(OT_CLI_VENDOR_EXTENSION "${SILABS_POSIX_CONFIG_PATH}/posix_vendor_cli.cmake" CACHE STRING "Name of cli extension cmake file")

# Compilation settings defined
set(OT_CONFIG openthread-core-silabs-posix-config.h CACHE STRING "Project-specific openthread config file")
set(OT_MULTIPAN_RCP ON CACHE BOOL "enable multi-PAN RCP")
set(OT_DIAGNOSTIC ON CACHE BOOL "enable diagnostic support")
set(OT_PACKAGE_NAME "SL-OPENTHREAD" CACHE STRING "OpenThread Package Name")
set(OT_FULL_LOGS OFF CACHE BOOL "Enable debug level logging" FORCE)

if(NOT BUILD_TESTING)
  set(ENABLE_CJSON_TEST OFF CACHE BOOL "ENABLE CJSON tests")
  set(ENABLE_TESTING OFF CACHE BOOL "Build mbed TLS tests.")
endif()

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/options.cmake")

add_subdirectory(${ot-br-posix_SOURCE_DIR} ${ot-br-posix_BINARY_DIR})

# Link vendor interface library directly to cpc
# as cpc is generated before otbr subdirectory 
target_link_libraries(rcp-vendor-intf PUBLIC cpc)
target_include_directories(rcp-vendor-intf PUBLIC ${cpcd_SOURCE_DIR}/lib)

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
  COMMAND sed -i "s|spinel.* |${OTBR_RADIO_URL} |" ${ot-br-posix_BINARY_DIR}/src/agent/otbr-agent.default
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
)

install(
  DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/debconf/services/otbr-firewall.service.d
  COMPONENT uic-otbr
  DESTINATION /etc/systemd/system
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
