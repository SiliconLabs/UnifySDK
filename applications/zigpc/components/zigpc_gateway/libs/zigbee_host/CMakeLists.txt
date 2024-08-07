# ##############################################################################
# Zigbee Host Wrapper Library
# ##############################################################################

include(ExternalProject)
find_package(GeckoSDK 2024.6.0 EXACT REQUIRED)
find_program(MAKE_EXECUTABLE NAMES make REQUIRED)

set(LIB_NAME libzigbee_host)
set(LIB_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/build)

# Definitions based on build type
if((NOT CMAKE_BUILD_TYPE) OR (CMAKE_BUILD_TYPE MATCHES Debug))
  set(LIB_MAKE_TARGET debug_lib)
  set(LIB_OUTPUT_DIR ${LIB_BUILD_DIR}/debug)
else()
  set(LIB_MAKE_TARGET release_lib)
  set(LIB_OUTPUT_DIR ${LIB_BUILD_DIR}/release)
endif()

set(LIB_PATH ${LIB_OUTPUT_DIR}/${LIB_NAME}.a)

set(SLC_PROJECT_FILE ${LIB_NAME}.slcp)
set(SLC_PROJECT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src/${SLC_PROJECT_FILE})
set(SLC_PROJECT_EXPORT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/gen)

# Scan for project build artifacts at cmake configure time (not build time) ----
# NOTE: to properly clean, run cmake before cleaning this component's artifacts:
# CMD1: `cmake ..` and CMD2: `ninja -t clean zigbee_host`
file(
  GLOB_RECURSE BUILD_BYPRODUCTS
  LIST_DIRECTORIES false
  "${SLC_PROJECT_EXPORT_PATH}/${LIB_NAME}*" "${LIB_BUILD_DIR}/*.o"
  "${LIB_BUILD_DIR}/*.d")

# Build Makefile project as a library
ExternalProject_Add(
  libzigbee_host
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
  BINARY_DIR ${SLC_PROJECT_EXPORT_PATH}
  BUILD_ALWAYS true
  UPDATE_COMMAND ""
  INSTALL_COMMAND ""
  CONFIGURE_COMMAND ""
  BUILD_BYPRODUCTS ${LIB_PATH} ${BUILD_BYPRODUCTS}
  BUILD_COMMAND
   ${MAKE_EXECUTABLE} BASE_SDK_PATH=${GeckoSDK_ROOT_DIR}
   BUILD_DIR=${LIB_BUILD_DIR} OUTPUT_DIR=${LIB_OUTPUT_DIR}
   CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER} LD=${CMAKE_LINKER}
   AR=${CMAKE_C_COMPILER_AR} C_FLAGS_DEBUG=${CMAKE_C_FLAGS_DEBUG}
   C_FLAGS_RELEASE=${CMAKE_C_FLAGS_RELEASE} AR_FLAGS=rs ${LIB_MAKE_TARGET})

# Add library that links to the Makefile generated library
set(ZIGBEE_HOST_STACK_INCLUDES
    ${GeckoSDK_ROOT_DIR}/protocol/zigbee
    ${GeckoSDK_ROOT_DIR}/protocol/zigbee/app/framework/common
    ${GeckoSDK_ROOT_DIR}/protocol/zigbee/stack
    ${GeckoSDK_ROOT_DIR}/platform/emlib/host/inc
    ${GeckoSDK_ROOT_DIR}/platform/radio/mac
    ${GeckoSDK_ROOT_DIR}/platform/service/legacy_hal/inc
    ${GeckoSDK_ROOT_DIR}/util/silicon_labs/silabs_core
    ${GeckoSDK_ROOT_DIR}/platform/common/inc)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads REQUIRED)

add_library(zigbee_host INTERFACE include/zigbee_host.h
            include/zigbee_host_callbacks.h)

add_dependencies(zigbee_host libzigbee_host)

target_link_libraries(zigbee_host INTERFACE ${LIB_PATH} ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(zigbee_host INTERFACE include gen/autogen gen/config
                                                 ${ZIGBEE_HOST_STACK_INCLUDES})
target_compile_definitions(
  zigbee_host INTERFACE EZSP_HOST=1 UC_BUILD=1 PLATFORM_HEADER="platform-header.h")

if(BUILD_TESTING)
  # Add mocks
  add_mock(zigbee_host_mock include/zigbee_host.h include/zigbee_host_callbacks.h)
  add_dependencies(zigbee_host_mock libzigbee_host)
  target_include_directories(
    zigbee_host_mock PRIVATE gen/autogen gen/config ${ZIGBEE_HOST_STACK_INCLUDES})
  target_compile_definitions(zigbee_host_mock
                            PUBLIC EZSP_HOST=1 UC_BUILD=1 PLATFORM_HEADER="platform-header.h")
  target_compile_options(zigbee_host_mock
                         PUBLIC
                         -Wno-address-of-packed-member)

  add_subdirectory(test)
endif()
