# SPDX-FileCopyrightText: Silicon Laboratories Inc. <https://www.silabs.com/>
# SPDX-License-Identifier: Zlib

# This recipe allows to download Unify Core
# Origin: https://github.com/SiliconLabs/UnifySDK/pulls?q=is%3Apr+author%3Arzr
# It can be used by projects which are depending on it
# Feel free to copy this (up to date) file everywhere it is needed

include(FetchContent)

# Standard cmake paths, you probably don't want/need to change
if(NOT DEFINED UNIFYSDK_SOURCE_DIR)
  set(UNIFYSDK_SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_deps/unifysdk-src")
endif()
if(NOT DEFINED UNIFYSDK_BINARY_DIR)
  set(UNIFYSDK_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/_deps/unifysdk-build") # TODO
endif()

if(NOT DEFINED UNIFYSDK_GIT_REPOSITORY)
  if(DEFINED ENV{UNIFYSDK_GIT_REPOSITORY})
    set(UNIFYSDK_GIT_REPOSITORY $ENV{UNIFYSDK_GIT_REPOSITORY})
  else()
    set(UNIFYSDK_GIT_REPOSITORY "")
  endif()
endif()
if(UNIFYSDK_GIT_REPOSITORY STREQUAL "")
  set(UNIFYSDK_GIT_REPOSITORY "https://github.com/SiliconLabs/UnifySDK")
endif()

if(NOT DEFINED UNIFYSDK_GIT_TAG)
  if(DEFINED ENV{UNIFYSDK_GIT_TAG})
    set(UNIFYSDK_GIT_TAG $ENV{UNIFYSDK_GIT_TAG})
  else()
    set(UNIFYSDK_GIT_TAG "")
  endif()
endif()

if(UNIFYSDK_GIT_TAG STREQUAL "")
  set(UNIFYSDK_GIT_TAG "main")
endif()

FetchContent_Declare(
  UnifySDK
  GIT_REPOSITORY ${UNIFYSDK_GIT_REPOSITORY}
  GIT_TAG        ${UNIFYSDK_GIT_TAG}
  GIT_SUBMODULES_RECURSE True
  GIT_SHALLOW 1
)

message(STATUS "${CMAKE_PROJECT_NAME}: Depends: ${UNIFYSDK_GIT_REPOSITORY}#${UNIFYSDK_GIT_TAG}")
message(STATUS "UnifySDK Sources: ${UNIFYSDK_SOURCE_DIR}")
message(STATUS "UnifySDK Binaries: ${UNIFYSDK_BINARY_DIR}")

# Pull only once, this has be refreshed by developer
if(NOT EXISTS ${CMAKE_BINARY_DIR}/_deps/unifysdk-src)
  if(UNIFYSDK_GIT_TAG STREQUAL "main")
    message(WARNING "Using: ${UNIFYSDK_GIT_TAG} TODO: Pin stable revision once supported")
  endif()
  FetchContent_MakeAvailable(UnifySDK)
else()
  message(STATUS "Using UnifySDK from previous fetch (may be outdated)")
  add_subdirectory(${CMAKE_BINARY_DIR}/_deps/unifysdk-src)
endif()
