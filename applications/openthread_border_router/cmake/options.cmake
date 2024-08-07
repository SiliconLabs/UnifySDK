# Options used to configure the otbr bootstrap and setup scripts
option(UIC_OTBR_BORDER_ROUTING "enable border routing features" 1)
option(UIC_OTBR_BACKBONE_ROUTER "enable backbone routing features" 1)
option(UIC_OTBR_REFERENCE_DEVICE "enable openthread reference device feature-set" 0)
option(UIC_OTBR_RELEASE "remove dependency on testing utilities" 1) # Unused - no runtime dependencies
option(UIC_OTBR_NAT64 "include nat64 service" 1)
option(UIC_OTBR_DNS64 "include dns64 service" 0)
option(UIC_OTBR_WEB_GUI "include web gui service" 1)
option(UIC_OTBR_REST_API "include rest api support" 1)
option(UIC_OTBR_DHCPV6_PD "include dhcp service" 0)
option(UIC_OTBR_NETWORK_MANAGER "include network manager service" 0)

# Set OTBR runtime and service options
set(OTBR_SYSTEMD_UNIT_DIR "/lib/systemd/system" CACHE STRING "Destination path for otbr systemd service files")
set(OTBR_INFRA_IF_NAME "eth0" CACHE STRING "The infrastructure interface name")
set(OTBR_RADIO_URL "spinel+cpc://cpcd_0?iid=2&iid-list=0" CACHE STRING "URL")
set(OTBR_MDNS "avahi" CACHE STRING "mDNS publisher provider")
set(OTBR_NAT64_SERVICE "openthread" CACHE STRING "nat64 service name")

# Packages installed in bootstrap when above options are enabled
set(UIC_OTBR_RECS "rsyslog, libavahi-client3, avahi-daemon, libjsoncpp-dev")
set(NAT64_RECS "iptables")
set(DNS64_RECS "bind9, resolvconf")
set(NETWORK_MANAGER_RECS "dnsmasq, network-manager")
set(DHCPV6_PD_RECS "dhcpcd5")
set(REFERENCE_DEVICE_RECS "radvd, dnsutils")
set(BACKBONE_ROUTER_RECS "libnetfilter-queue1")
set(WEB_GUI_RECS "nodejs, npm")

# Including options to enable and packages to recommend
# based off of selected options
if(UIC_OTBR_NAT64)
    string(COMPARE EQUAL ${OTBR_NAT64_SERVICE} "tayga" TAYGA_NAT64_SERVICE)

    if(TAYGA_NAT64_SERVICE)
        string(APPEND NAT64_RECS ", ${OTBR_NAT64_SERVICE}")
    endif(TAYGA_NAT64_SERVICE)

    string(APPEND UIC_OTBR_RECS ", ${NAT64_RECS}")
endif(UIC_OTBR_NAT64)

if(UIC_OTBR_DNS64)
    string(APPEND UIC_OTBR_RECS ", ${DNS64_RECS}")
endif(UIC_OTBR_DNS64)

if(UIC_OTBR_NETWORK_MANAGER)
    string(APPEND UIC_OTBR_RECS ", ${NETWORK_MANAGER_RECS}")
endif(UIC_OTBR_NETWORK_MANAGER)

if(UIC_OTBR_DHCPV6_PD)
    string(APPEND UIC_OTBR_RECS ", ${DHCPV6_PD_RECS}")
endif(UIC_OTBR_DHCPV6_PD)

if(UIC_OTBR_REFERENCE_DEVICE)
    string(APPEND UIC_OTBR_RECS ", ${REFERENCE_DEVICE_RECS}")
    set(OTBR_NO_AUTO_ATTACH 1 CACHE BOOL "disable auto Thread attach")
    set(OT_REFERENCE_DEVICE ON CACHE BOOL "enable openthread reference device feature-set")
    set(OT_DHCP6_CLIENT ON CACHE BOOL "enable DHCP6 client support")
    set(OT_DHCP6_SERVER ON CACHE BOOL "enable DHCP6 server support")
endif(UIC_OTBR_REFERENCE_DEVICE)

if(UIC_OTBR_BACKBONE_ROUTER)
    string(APPEND UIC_OTBR_RECS ", ${BACKBONE_ROUTER_RECS}")
    set(OTBR_BACKBONE_ROUTER ON CACHE BOOL "enable backbone router features")
    if(UIC_OTBR_REFERENCE_DEVICE)
        set(OTBR_DUA_ROUTING ON CACHE BOOL "enable backbone router DUA routing")
    endif(UIC_OTBR_REFERENCE_DEVICE)
endif(UIC_OTBR_BACKBONE_ROUTER)

if(UIC_OTBR_BORDER_ROUTING)
    set(OTBR_BORDER_ROUTING ON CACHE BOOL "enable border routing features")
endif(UIC_OTBR_BORDER_ROUTING)

if(UIC_OTBR_WEB_GUI)
    string(APPEND UIC_OTBR_RECS ", ${WEB_GUI_RECS}")
    set(OTBR_WEB ON CACHE BOOL "enable web gui service")
endif(UIC_OTBR_WEB_GUI)

if(UIC_OTBR_REST_API)
    set(OTBR_REST ON CACHE BOOL "enable rest api support")
    set(ENABLE_CJSON_TEST OFF CACHE BOOL "enable cJSON lib testing" FORCE) # External unity dependency creates cmake target name conflict
endif(UIC_OTBR_REST_API)

# Setting additional default options
option(OTBR_DBUS "enable DBUS support" ON)
option(OTBR_DNSSD_DISCOVERY_PROXY "enable dns-sd discovery proxy support" ON)
option(OTBR_SRP_ADVERTISING_PROXY "enable advertising proxy" ON)

# Adding selected packages to debian recommends control field
set(CPACK_DEBIAN_UIC-OTBR_PACKAGE_RECOMMENDS ${UIC_OTBR_RECS} CACHE STRING "Package recommendations for uic-otbr: ${UIC_OTBR_RECS}" FORCE)
