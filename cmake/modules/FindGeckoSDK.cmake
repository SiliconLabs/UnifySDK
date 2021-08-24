# Find the Silicon Labs Gecko SDK location
# =============================================================================
# Usage of this module is as follows:
#
# find_package(GeckoSDK)
#
# Variables used by this module:
# * ENV variable GSDK_LOCATION is used to find the Gecko SDK path
#
# Variables defined by this module:
# * GeckoSDK_FOUND               - True if the Gecko SDK location is found
# * GeckoSDK_VERSION             - Version of the Gecko SDK found
# * GeckoSDK_EMBERZNET_VERSION   - Version of the Zigbee stack found
# * GeckoSDK_ROOT_DIR            - Path to the Gecko SDK base directory
#
# =============================================================================


# Look for EmberZNet stack.properties file that exist in every release
unset(GeckoSDK_ROOT_DIR CACHE)
find_path(
  GeckoSDK_ROOT_DIR
  NAMES stack.properties
  PATHS "$ENV{GSDK_LOCATION}/protocol/zigbee")
mark_as_advanced(GeckoSDK_ROOT_DIR)

if(${GeckoSDK_ROOT_DIR} MATCHES "GeckoSDK_ROOT_DIR-NOTFOUND")
  message(
    WARNING
      "Gecko SDK NOT found. Set ENV variable GSDK_LOCATION to correct path")
else()
  set(GeckoSDK_ROOT_DIR $ENV{GSDK_LOCATION})
  # Find the version of the Gecko SDK used based on possible release file
  # locations
  if(EXISTS
     "${GeckoSDK_ROOT_DIR}/protocol/zigbee/documentation/release-highlights.txt"
  )
    set(_emberznet_release_file "documentation/release-highlights.txt")
  elseif(EXISTS
         "${GeckoSDK_ROOT_DIR}/protocol/zigbee/docs/release-highlights.txt")
    set(_emberznet_release_file "docs/release-highlights.txt")
  endif()

  if(NOT _emberznet_release_file)
    message(
      WARNING
        "Unable to find EmberZNet version from Gecko SDK release notes. Setting version to 0.0.0.0"
    )
    set(GeckoSDK_EMBERZNET_VERSION "0.0.0.0")
  else()
    file(
      STRINGS ${GeckoSDK_ROOT_DIR}/protocol/zigbee/${_emberznet_release_file}
      _ver_line
      REGEX "EmberZNet.+[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+"
      LIMIT_COUNT 1)

    string(REGEX MATCH "[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+"
                 GeckoSDK_EMBERZNET_VERSION "${_ver_line}")
  endif()
  unset(_emberznet_release_file)

  # Convert EmberZNet version to GSDK version
  if(${GeckoSDK_EMBERZNET_VERSION} MATCHES "6\.7\.[0-9]+\.[0-9]+")
    set(GeckoSDK_VERSION "2.7")
  elseif(${GeckoSDK_EMBERZNET_VERSION} MATCHES "6\.8\.[0-9]+\.[0-9]+")
    set(GeckoSDK_VERSION "3.0")
  elseif(${GeckoSDK_EMBERZNET_VERSION} MATCHES "6\.9\.[0-9]+\.[0-9]+")
    set(GeckoSDK_VERSION "3.1")
  else()
    set(GeckoSDK_VERSION "0.0")
  endif()

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(
    GeckoSDK
    REQUIRED_VARS GeckoSDK_ROOT_DIR GeckoSDK_VERSION
    VERSION_VAR GeckoSDK_VERSION)
endif()
