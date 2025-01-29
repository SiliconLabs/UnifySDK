# SPDX-FileCopyrightText: Silicon Laboratories Inc. <https://www.silabs.com/>
# SPDX-License-Identifier: Zlib
#
# Origin: https://github.com/SiliconLabs/UnifySDK/pull/51
#
# This recipe allows to download Unify Core
# It can be used by projects which are depending on it
# Feel free to copy this (up to date) file everywhere it is needed

include(FetchContent)

if(NOT DEFINED UNIFYSDK_GIT_REPOSITORY)
  if(DEFINED ENV{UNIFYSDK_GIT_REPOSITORY})
    set(UNIFYSDK_GIT_REPOSITORY $ENV{UNIFYSDK_GIT_REPOSITORY})
  endif()
endif()
if("${UNIFYSDK_GIT_REPOSITORY}" STREQUAL "")
  set(UNIFYSDK_GIT_REPOSITORY "https://github.com/SiliconLabs/UnifySDK")
endif()

if(NOT DEFINED UNIFYSDK_GIT_TAG)
  if(DEFINED ENV{UNIFYSDK_GIT_TAG})
    set(UNIFYSDK_GIT_TAG $ENV{UNIFYSDK_GIT_TAG})
  endif()
endif()
if("${UNIFYSDK_GIT_TAG}" STREQUAL "")
  set(UNIFYSDK_GIT_TAG "main") # Override CMake default ("master")
endif()

FetchContent_Declare(
  UnifySDK
  GIT_REPOSITORY ${UNIFYSDK_GIT_REPOSITORY}
  GIT_TAG        ${UNIFYSDK_GIT_TAG}
  GIT_SUBMODULES_RECURSE True
  GIT_SHALLOW 1
)

message(STATUS "${CMAKE_PROJECT_NAME}: Depends: ${UNIFYSDK_GIT_REPOSITORY}#${UNIFYSDK_GIT_TAG}")
string(REGEX MATCH ".*/?main/?.*" UNIFYSDK_UNSTABLE_GIT_TAG "${UNIFYSDK_GIT_TAG}")
if(UNIFYSDK_GIT_TAG STREQUAL "" OR UNIFYSDK_UNSTABLE_GIT_TAG)
  message(WARNING "${CMAKE_PROJECT_NAME}: Declare UNIFYSDK_GIT_TAG to stable version not: ${UNIFYSDK_UNSTABLE_GIT_TAG}")
endif()

set(FETCHCONTENT_QUIET FALSE)
FetchContent_MakeAvailable(UnifySDK)

# message(STATUS "UnifySDK Sources: ${unifysdk_SOURCE_DIR}")
# message(STATUS "UnifySDK Binaries: ${unifysdk_BINARY_DIR}")
