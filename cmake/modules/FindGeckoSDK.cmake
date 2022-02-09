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
# * GeckoSDK_ROOT_DIR            - Path to the Gecko SDK base directory
#
# =============================================================================


# Look for gecko_sdk.slcs file that exist in every release
unset(GeckoSDK_ROOT_DIR CACHE)
find_path(
  GeckoSDK_ROOT_DIR
  NAMES gecko_sdk.slcs
  PATHS "$ENV{GSDK_LOCATION}" )
mark_as_advanced(GeckoSDK_ROOT_DIR)

if(${GeckoSDK_ROOT_DIR} MATCHES "GeckoSDK_ROOT_DIR-NOTFOUND")
  message(
    WARNING
      "Gecko SDK NOT found. Set ENV variable GSDK_LOCATION to correct path")
else()
  # Find the version of the Gecko SDK used based on SLC metafile
  file(
    STRINGS ${GeckoSDK_ROOT_DIR}/gecko_sdk.slcs
    _ver_line
    REGEX "sdk_version: +\"[0-9\.]+\""
    LIMIT_COUNT 1)
  string(REGEX MATCH "[0-9\.]+" GeckoSDK_VERSION "${_ver_line}")

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(
    GeckoSDK
    REQUIRED_VARS GeckoSDK_ROOT_DIR GeckoSDK_VERSION
    VERSION_VAR GeckoSDK_VERSION)
endif()
