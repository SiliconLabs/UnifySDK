# Developer GUI install
find_program(NPM_EXECUTABLE "npm" REQUIRED)

if(APPLE)
  set(NPM_TARGET macos-x64)
elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
  set(NPM_TARGET linux-x64)
elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "armhf")
  set(NPM_TARGET linux-armv7)
else()
  message(ERROR "Unsuported target")
endif()

run_zap(zap/gen-templates.json)

file(GLOB GUI_SRCS .)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/package-lock.json
  DEPENDS
    ${GUI_SRCS}
    ${CMAKE_CURRENT_BINARY_DIR}/src/cluster-types/cluster-type-attributes.ts
    ${CMAKE_CURRENT_BINARY_DIR}/src/cluster-types/cluster-types.ts
    ${CMAKE_CURRENT_BINARY_DIR}/src/cluster-types/supported-clusters.js
  COMMAND cp -r ${CMAKE_CURRENT_SOURCE_DIR}/. .
  COMMAND ${NPM_EXECUTABLE} version
          ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REV}
  COMMAND ${NPM_EXECUTABLE} install --legacy-peer-deps # With yarn 38s with npm 69s change
  COMMAND ${NPM_EXECUTABLE} run build
  USES_TERMINAL)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/dev-gui-client
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/package-lock.json
  COMMAND TARGET=${NPM_TARGET} ${NPM_EXECUTABLE} run dev-gui-client-build
  USES_TERMINAL)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/dev-gui-api
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/dev-gui-client
  COMMAND TARGET=${NPM_TARGET} ${NPM_EXECUTABLE} run dev-gui-api-build
  USES_TERMINAL)

separate_arguments(ZAP_GENERATE_CMD_ARGS NATIVE_COMMAND ${ZAP_GENERATE_CMD_STR})

add_custom_target(
  dev_gui ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/dev-gui-api # Will also trigger
                                                  # dev-gui-client build
)

# Package as .debian
set(description "Universal IOT controller - Developer GUI")
cpack_add_component(
  uic-dev-gui
  DISPLAY_NAME "dev-gui"
  DESCRIPTION ${description}
  INSTALL_TYPES Full)

install(
  FILES assets/cert.pem assets/key.pem
  DESTINATION /var/lib/dev-gui
  COMPONENT uic-dev-gui)

install(
  FILES debconf/services/uic-dev-gui-api.service
  DESTINATION /lib/systemd/system/
  COMPONENT uic-dev-gui)
install(
  FILES debconf/services/uic-dev-gui-client.service
  DESTINATION /lib/systemd/system/
  COMPONENT uic-dev-gui)

# Install binaries they come from custom target build_dev_gui
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/../dev-gui-api
  DESTINATION "bin"
  PERMISSIONS
    OWNER_READ
    OWNER_WRITE
    OWNER_EXECUTE
    GROUP_EXECUTE
    GROUP_READ
    WORLD_READ
    WORLD_EXECUTE
  COMPONENT uic-dev-gui)
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/../dev-gui-client
  DESTINATION "bin"
  PERMISSIONS
    OWNER_READ
    OWNER_WRITE
    OWNER_EXECUTE
    GROUP_EXECUTE
    GROUP_READ
    WORLD_READ
    WORLD_EXECUTE
  COMPONENT uic-dev-gui)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/scripts/prerm
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/prerm)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/scripts/postinst
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/postinst)
