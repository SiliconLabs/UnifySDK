include(FetchContent)
find_package(GeckoSDK 4.1.1 REQUIRED)
find_package(CPCD 4.1.1 REQUIRED)

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

file(GLOB CPC_HEADERS "${cpcd_SOURCE_DIR}/lib/sl_cpc.h")
file(COPY ${CPC_HEADERS} DESTINATION ${openthread_SOURCE_DIR}/src/posix/platform)

# Compilation settings defined
set(OT_MULTIPAN_RCP ON CACHE BOOL "enable multi-PAN RCP")
set(OT_POSIX_CONFIG_RCP_BUS "CPC" CACHE STRING "RCP bus type")
set(OTBR_INFRA_IF_NAME "eth0" CACHE STRING "The infrastructure interface name")
set(OT_DIAGNOSTIC ON CACHE BOOL "enable diagnostic support")
set(OT_PACKAGE_NAME "SL-OPENTHREAD" CACHE STRING "OpenThread Package Name")
set(OTBR_MDNS "avahi" CACHE STRING "mDNS publisher provider")

# TODO: This is recommended settings but breaks compilation
# set(OT_DUA ON CACHE BOOL "enable Domain Unicast Address feature for Thread 1.2")
# set(OTBR_DUA_ROUTING ON CACHE BOOL "Enable Backbone Router DUA Routing")

# Default not to debug log level
set(OT_LOG_LEVEL "NONE" CACHE STRING "set OpenThread log level")
set(OT_FULL_LOGS OFF CACHE BOOL "Enable debug level logging" FORCE)

add_subdirectory(${ot-br-posix_SOURCE_DIR} ${ot-br-posix_BINARY_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/prerm.in
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/prerm)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/postinst.in
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/postinst)

# Remove otbr compilation for target setup
execute_process(
  COMMAND sed -i "/otbr/d" ${ot-br-posix_SOURCE_DIR}/script/setup
  COMMAND sed -i "s/set -euxo pipefail/set -euo pipefail/g" ${ot-br-posix_SOURCE_DIR}/script/_initrc
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

add_component_to_uic(
  uic-otbr # Package Name
  "OpenThread Border Router"
  "${CMAKE_PROJECT_NAME}-otbr" # Package Debian filename
  "libavahi-client3, avahi-daemon, ipset, uic-cpcd"
  "${CMAKE_CURRENT_BINARY_DIR}/debconf/postinst;\
   ${CMAKE_CURRENT_BINARY_DIR}/debconf/prerm;"
)
