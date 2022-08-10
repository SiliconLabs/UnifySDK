# Find the Silicon Labs CPCd location
# =============================================================================
# Usage of this module is as follows:
#
# find_package(CPCD)
#
# cmake-format: off
# Variables used by this module:
#
# Variables defined by this module:
# * CPCD_FOUND - True if the CPCd sources are found.
# cmake-format: on
# =============================================================================
include(FetchContent)

if(DEFINED ENV{CPCD_LOCATION})
  set(FETCHCONTENT_SOURCE_DIR_CPCD "$ENV{CPCD_LOCATION}")
  message(STATUS "CPCd - using provided CPCD_LOCATION ($ENV{CPCD_LOCATION})")
endif()

if(NOT DEFINED FETCH_CPCD_VERSION)
  # The version to fetch should ideally be the same as the version used for fetching GeckoSDK
  set(FETCH_CPCD_VERSION "v4.1.0")
endif()

set(FETCHCONTENT_QUIET FALSE)
FetchContent_Declare(
  cpcd
  GIT_REPOSITORY "https://github.com/SiliconLabs/cpc-daemon"
  GIT_SUBMODULES_RECURSE True
  GIT_SHALLOW True
  GIT_PROGRESS True
  GIT_TAG ${FETCH_CPCD_VERSION}
  SOURCE_DIR "${CMAKE_SOURCE_DIR}/externals/cpcd"
  SUBBUILD_DIR "${CMAKE_SOURCE_DIR}/externals/cache/cpcd_subbuild"
)
FetchContent_MakeAvailable(CPCD)

message(STATUS "cpcd_SOURCE_DIR: ${cpcd_SOURCE_DIR}")

# Find the version of CPCd
file(
  STRINGS ${cpcd_SOURCE_DIR}/CMakeLists.txt _ver_line
  REGEX "project\\(cpcd VERSION \"([^\"]*)\""
  LIMIT_COUNT 1)
string(REGEX MATCH "[0-9\.]+" CPCD_VERSION "${_ver_line}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  CPCD
  REQUIRED_VARS CPCD_VERSION
  VERSION_VAR CPCD_VERSION)

