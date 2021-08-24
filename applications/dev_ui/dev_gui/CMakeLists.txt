# Developer GUI install
find_program(NPM_EXECUTABLE "npm" REQUIRED)
find_program(YARN_EXECUTABLE "yarn" REQUIRED)

if(NOT NPM_EXECUTABLE OR NOT YARN_EXECUTABLE)
  if(NOT NPM_EXECUTABLE AND NOT YARN_EXECUTABLE)
    message(STATUS "Could NOT find npm and yarn")
  elseif(NOT NPM_EXECUTABLE)
    message(STATUS "Could NOT find npm")
  else()
    message(STATUS "Could NOT find yarn")
  endif()
  message(STATUS "Developer GUI build skipped")
else()
  message(STATUS "Developer GUI will build")

  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/package-lock.json
    DEPENDS
      src/
      package.json
      public
      tsconfig.json
      ${CMAKE_CURRENT_BINARY_DIR}/src/cluster-types/cluster-type-attributes.ts
      ${CMAKE_CURRENT_BINARY_DIR}/src/cluster-types/cluster-types.ts
      ${CMAKE_CURRENT_BINARY_DIR}/src/cluster-types/supported-clusters.js
    COMMAND cp -r ${CMAKE_CURRENT_SOURCE_DIR}/src .
    COMMAND cp -r ${CMAKE_CURRENT_SOURCE_DIR}/package.json .
    COMMAND
      sed -i -e
      's/\"version\": \"0.0.0\"/\"version\": \"${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REV}\"/'
      ${CMAKE_CURRENT_BINARY_DIR}/package.json
    COMMAND cp -r ${CMAKE_CURRENT_SOURCE_DIR}/public .
    COMMAND cp -r ${CMAKE_CURRENT_SOURCE_DIR}/tsconfig.json .
    COMMAND ${NPM_EXECUTABLE} install --legacy-peer-deps --loglevel silent >
            /dev/null # With yarn 38s with npm 69s change
    COMMAND ${NPM_EXECUTABLE} run build > /dev/null)

  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/dev-gui-server
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/package-lock.json
    COMMAND ${NPM_EXECUTABLE} run
            dev-gui-server-build-${CMAKE_SYSTEM_PROCESSOR})

  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/dev-gui-api
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/dev-gui-server
    COMMAND ${NPM_EXECUTABLE} run dev-gui-api-build-${CMAKE_SYSTEM_PROCESSOR})

  add_custom_command(
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/src/cluster-types/cluster-type-attributes.ts
      ${CMAKE_CURRENT_BINARY_DIR}/src/cluster-types/cluster-types.ts
      ${CMAKE_CURRENT_BINARY_DIR}/src/cluster-types/supported-clusters.js
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/zap/cluster-type-attributes.zapt
            ${CMAKE_CURRENT_SOURCE_DIR}/zap/cluster-types.zapt
            ${CMAKE_CURRENT_SOURCE_DIR}/zap/supported-clusters.zapt
            ${CMAKE_CURRENT_SOURCE_DIR}/zap/gen-templates.json
            ${CMAKE_CURRENT_SOURCE_DIR}/zap/addon-helper.js
    COMMAND
      ${ZAP_EXECUTABLE} generate --noUi --noServer --no-sandbox -z
      ../../../../components/uic_dotdot/dotdot-xml/library.xml -g
      ${CMAKE_CURRENT_SOURCE_DIR}/zap/gen-templates.json -o
      ${CMAKE_CURRENT_BINARY_DIR}/src/cluster-types)

  add_custom_target(
    dev_gui ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/dev-gui-api # Will also trigger
                                                    # dev-gui-server build
  )

  # Package as .debian
  set(description "Universal IOT controller - Developer GUI")
  cpack_add_component(
    uic-dev-gui
    DISPLAY_NAME "dev-gui"
    DESCRIPTION ${description}
    INSTALL_TYPES Full)

  install(
    DIRECTORY
    DESTINATION /var/lib/dev-gui
    COMPONENT uic-dev-gui)

  install(
    FILES debconf/services/uic-dev-gui-api.service
    DESTINATION /lib/systemd/system/
    COMPONENT uic-dev-gui)
  install(
    FILES debconf/services/uic-dev-gui-server.service
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
    FILES ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/../dev-gui-server
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
endif()
