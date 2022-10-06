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
  DEPENDS ${GUI_SRCS}
          ${ZAP_CURRENT_OUTPUT_DIR}/src/cluster-types/cluster-type-attributes.ts
          ${ZAP_CURRENT_OUTPUT_DIR}/src/cluster-types/cluster-types.ts
          ${ZAP_CURRENT_OUTPUT_DIR}/src/cluster-types/supported-clusters.js
  COMMAND cp -r ${CMAKE_CURRENT_SOURCE_DIR}/. .
  COMMAND cp -r ${ZAP_CURRENT_OUTPUT_DIR}/. .
  COMMAND ${NPM_EXECUTABLE} version --allow-same-version 
          ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REV}
  COMMAND ${NPM_EXECUTABLE} install --legacy-peer-deps --allow-same-version # With yarn 38s with npm 69s change
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

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/public/index.html.in
               ${CMAKE_CURRENT_BINARY_DIR}/public/index.html)

if("${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr")
  # Package as .debian
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
endif()

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

add_component_to_uic(
  uic-dev-gui
  "Universal IOT controller - Developer GUI"
  "${CMAKE_PROJECT_NAME}-dev-gui"
  ""
  "${CMAKE_BINARY_DIR}/applications/dev_ui/dev_gui/debconf/postinst;\
    ${CMAKE_BINARY_DIR}/applications/dev_ui/dev_gui/debconf/prerm;\
    ${CMAKE_BINARY_DIR}/applications/dev_ui/dev_gui/debconf/postrm")
