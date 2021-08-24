# -------------------------------------------------------------------------------------------------
# Z3Gateway Interface component for Appbuilder Makefile library
# -------------------------------------------------------------------------------------------------

include(ExternalProject)
find_package(GeckoSDK 3.1 EXACT REQUIRED)
find_program(MAKE_EXE NAMES make)

set(LIBZ3GATEWAY_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/bin)

# Makefile generated library path
set(LIBZ3GATEWAY_FILE ${LIBZ3GATEWAY_OUTPUT_DIR}/exe/z3gw.a)

# List of Makefile generated object files
file(GLOB LIBZ3GATEWAY_OBJ_FILES "${LIBZ3GATEWAY_OUTPUT_DIR}/*.o"
     "${LIBZ3GATEWAY_OUTPUT_DIR}/*.d")

# Build Makefile project as a library
ExternalProject_Add(
  libz3gateway
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
  BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}
  BUILD_ALWAYS true
  CONFIGURE_COMMAND ""
  INSTALL_COMMAND ""
  UPDATE_COMMAND ""
  BUILD_BYPRODUCTS ${LIBZ3GATEWAY_FILE} ${LIBZ3GATEWAY_OBJ_FILES}
  BUILD_COMMAND
    ${MAKE_EXE} COMPILER=${CMAKE_C_COMPILER}
    OUTPUT_DIR=${LIBZ3GATEWAY_OUTPUT_DIR} LINKER=${CMAKE_C_LINKER}
    ARCHIVE=${CMAKE_C_COMPILER_AR} NO_READLINE=1
    GSDK_LOCATION=${GeckoSDK_ROOT_DIR} GENERATE_LIBRARY=1 --silent)

# Add library that links to the Makefile generated library
set(Z3GATEWAY_GSDK_INCLUDES
    ${GeckoSDK_ROOT_DIR}/protocol/zigbee
    ${GeckoSDK_ROOT_DIR}/protocol/zigbee/stack
    ${GeckoSDK_ROOT_DIR}/platform/radio/mac
    ${GeckoSDK_ROOT_DIR}/util/silicon_labs/silabs_core)

add_library(z3gateway INTERFACE include/z3gateway.h
            include/z3gateway_callbacks.h)
add_dependencies(z3gateway libz3gateway)
target_link_libraries(z3gateway INTERFACE ${LIBZ3GATEWAY_FILE})
target_include_directories(z3gateway INTERFACE include gen
                                               ${Z3GATEWAY_GSDK_INCLUDES})

# Add mocks
add_mock(z3gateway_mock include/z3gateway.h include/z3gateway_callbacks.h)
target_include_directories(z3gateway_mock PRIVATE gen
                                                  ${Z3GATEWAY_GSDK_INCLUDES})

add_subdirectory(test)
