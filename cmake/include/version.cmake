if (NOT VERSION_CMAKE_FILE)
  set(VERSION_CMAKE_FILE ${CMAKE_CURRENT_LIST_DIR})
else()
  return()
endif()

# CMAKE_PROJECT_VERSION AND GIT_VERSION logic.
# 1. If ${CMAKE_CURRENT_SOURCE_DIR}/cmake/release-version.cmake is present
# (For source zip file)
#    GIT_VERSION will be set in that
#
#  else  GIT_VERSION is be based on "git describe"
#  else if git describe not available then GIT_VERSION will be ver_0.0.0-0-xxxx
#
# 2. if (GIT_VERSION does not match the patter ver_x.x.x)
#     then: look for previous tag that matches this pattern.
#           set GIT_VERSION to Previous_Tag-$GIT_VERSION
#
#    else()
#      set GIT_VERSION to ver_0.0.0
#
# 3. find MAJOR MINOR REV PATCH from $GIT_VERSION
#  set CMAKE_PROJECT_VERSION to MAJOR.MINOR.REV
#
# Generate GIT_VERSION from "git describe"
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/../release-version.cmake")
  include(${CMAKE_CURRENT_LIST_DIR}/../release-version.cmake)
  MESSAGE(STATUS "GIT_VERSION from cmake/release-version.cmake: "
          ${GIT_VERSION})
endif()

if ("${GIT_VERSION}" STREQUAL "")
  find_package(Git)
  if(Git_FOUND)
    message("Git found: ${GIT_EXECUTABLE}")
    execute_process (
        COMMAND ${GIT_EXECUTABLE} describe
        OUTPUT_VARIABLE GIT_VERSION
        ERROR_QUIET
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
    if ("${GIT_VERSION}" STREQUAL "")
      execute_process (
          COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
          OUTPUT_VARIABLE GIT_VERSION_SHA1
          ERROR_QUIET
      )
      string(CONCAT GIT_VERSION "ver_0.0.0-" ${GIT_VERSION_SHA1})
    endif()

  endif()
  if ("${GIT_VERSION}" STREQUAL "")
    # If both git describe and release-version.cmake are not available
    # For e.g. if someone ACCIDENTALY does make package_source instead of make
    # uic_source and tries to build that source zip file
    set(GIT_VERSION "ver_0.0.0-0-xxxx")
    message(WARNING "${DIR_OF_VERSION_CMAKE}/cmake/release-version.cmake "
                    " and "
                    "git describe not available")
  else()
    string(REGEX REPLACE "\n$" "" GIT_VERSION "${GIT_VERSION}")
  endif()
  MESSAGE(STATUS "GIT_VERSION from git describe is: " ${GIT_VERSION})
endif()

# If GIT_VERSION is not in format ver_1.2.3
string(REGEX MATCH "^ver" OUT "${GIT_VERSION}")
if ("${OUT}" STREQUAL "")
  message (STATUS "Invalid tag format: ${GIT_VERSION}. It should be ver_<major>"
           ".<minor>.<revision> For e.g. ver_1.2.3")
  if(Git_FOUND)
    MESSAGE(STATUS "Will try to find previous tag matching this pattern")
    execute_process(
      COMMAND git describe --match "ver_[0-9].[0-9].[0-9]*"  HEAD
      OUTPUT_VARIABLE GIT_VERSION_PREV
      ERROR_QUIET
      )
    string(REGEX REPLACE "\n$" "" GIT_VERSION_PREV "${GIT_VERSION_PREV}")
    message (STATUS "Found: " ${GIT_VERSION_PREV})
    string(CONCAT GIT_VERSION "${GIT_VERSION_PREV}" "-" "${GIT_VERSION}")
  else() #Very unlikely
    set(GIT_VERSION "ver_0.0.0")
    message (WARNING "${GIT_VERSION}")
  endif()
endif()

# Sanitize the patch component of the symantic versioning. According Debian
# spec only  + . ~ (plus, full stop, tilde) are allowed inside the patch
# version. We should update our code to align with the Debian spec
# (simplicity) untill then, we sanitize the patch version.
string(REGEX MATCH "(^ver_[0-9]+.[0-9]+.[0-9]+-?)(.*)" GIT_VERSION "${GIT_VERSION}")
if (${CMAKE_MATCH_2})
  string(REPLACE "_" "-" CMAKE_MATCH_2 ${CMAKE_MATCH_2})
endif()
set(GIT_VERSION ${CMAKE_MATCH_1}${CMAKE_MATCH_2})
message (STATUS "Build number will be: " ${GIT_VERSION})

#Generate MAJOR MINOR REV and PATCH from GIT_VERSION for CMAKE_PROJECT_VERSION
string(REGEX REPLACE "^ver_([0-9]+).*" "\\1" VERSION_MAJOR "${GIT_VERSION}")
string(REGEX REPLACE "^ver_[0-9]+.([0-9]+).*" "\\1" VERSION_MINOR "${GIT_VERSION}")
string(REGEX REPLACE "^ver_[0-9]+.[0-9]+.([0-9]+).*" "\\1" VERSION_REV "${GIT_VERSION}")
string(REGEX REPLACE "^ver_[0-9]+.[0-9]+.[0-9]+-?(.*)" "\\1" VERSION_PATCH "${GIT_VERSION}")

# Generating version info file for CI and release packaging
configure_file(${CMAKE_CURRENT_LIST_DIR}/../version-info.json.in
               ${CMAKE_CURRENT_BINARY_DIR}/cmake/version-info.json)

message(STATUS "MAJOR: " ${VERSION_MAJOR})
message(STATUS "MINOR: " ${VERSION_MINOR})
message(STATUS "REVISION: " ${VERSION_REV})
message(STATUS "PATCH: " ${VERSION_PATCH})

