find_package(GeckoSDK 4.3.0 REQUIRED)
find_package(CPCD 4.3.0 REQUIRED)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
find_program(MAKE_EXECUTABLE NAMES make REQUIRED)

set(BGBUILD ${GeckoSDK_ROOT_DIR}/protocol/bluetooth/bin/gatt/bgbuild.py)

set(EXPORT_DIR ${CMAKE_CURRENT_BINARY_DIR}/export)
set(AUTOGEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/autogen)
set(BTCONF_DIR ${EXPORT_DIR}/app/bluetooth/example_host/bt_host_empty/config/btconf)
set(BTCONF_FILE ${BTCONF_DIR}/gatt_configuration.btconf)

set(BT_HOST_EMPTY_SRC_FILES
  ${EXPORT_DIR}/app/common/util/app_log/app_log.c
  ${EXPORT_DIR}/app/bluetooth/common_host/app_log/app_log_cli.c
  ${EXPORT_DIR}/app/bluetooth/common_host/iostream_mock/sl_iostream_handles.c
  ${EXPORT_DIR}/app/bluetooth/common_host/app_signal/app_signal_posix.c
  ${EXPORT_DIR}/app/bluetooth/common_host/app_sleep/app_sleep.c
  ${EXPORT_DIR}/app/bluetooth/common_host/host_comm/host_comm_posix.c
  ${EXPORT_DIR}/app/bluetooth/common_host/ncp_host/ncp_host.c
  ${EXPORT_DIR}/app/bluetooth/common_host/tcp/tcp_posix.c
  ${EXPORT_DIR}/app/bluetooth/common_host/uart/uart_posix.c
  ${EXPORT_DIR}/app/bluetooth/common_host/cpc/cpc.c
  ${EXPORT_DIR}/protocol/bluetooth/src/sl_bt_ncp_host.c
  ${EXPORT_DIR}/protocol/bluetooth/src/sl_bt_ncp_host_api.c
  ${EXPORT_DIR}/app/bluetooth/common_host/named_socket/named_socket.c
  ${EXPORT_DIR}/app/bluetooth/common/ncp_gatt/sl_ncp_gatt.c
  ${EXPORT_DIR}/app/bluetooth/common_host/ncp_version/ncp_version.c
  ${EXPORT_DIR}/app/bluetooth/common_host/system/system.c
  ${EXPORT_DIR}/app/bluetooth/example_host/bt_host_empty/app.c
  ${EXPORT_DIR}/app/bluetooth/example_host/bt_host_empty/main.c
)

set(BT_HOST_EMPTY_INCLUDES
  ${EXPORT_DIR}/app/common/util/app_log
  ${EXPORT_DIR}/app/bluetooth/common_host/iostream_mock
  ${EXPORT_DIR}/app/bluetooth/common_host/app_log
  ${EXPORT_DIR}/app/bluetooth/common_host/app_log/config
  ${EXPORT_DIR}/app/common/util/app_assert
  ${EXPORT_DIR}/app/bluetooth/common_host/app_assert/config
  ${EXPORT_DIR}/app/bluetooth/common_host/app_signal
  ${EXPORT_DIR}/app/bluetooth/common_host/app_sleep
  ${EXPORT_DIR}/app/bluetooth/common_host/host_comm
  ${EXPORT_DIR}/app/bluetooth/common_host/host_comm/config
  ${EXPORT_DIR}/app/bluetooth/common_host/ncp_host
  ${EXPORT_DIR}/app/bluetooth/common_host/ncp_host/config
  ${EXPORT_DIR}/app/bluetooth/common_host/system
  ${EXPORT_DIR}/app/bluetooth/common_host/tcp
  ${EXPORT_DIR}/app/bluetooth/common_host/uart
  ${EXPORT_DIR}/app/bluetooth/common_host/cpc
  ${EXPORT_DIR}/platform/common/inc
  ${EXPORT_DIR}/protocol/bluetooth/inc
  ${EXPORT_DIR}/app/bluetooth/common_host/named_socket
  ${EXPORT_DIR}/app/bluetooth/common/ncp_gatt
  ${EXPORT_DIR}/app/bluetooth/common_host/ncp_version
  ${EXPORT_DIR}/app/bluetooth/example_host/bt_host_empty
)

add_custom_command(
  OUTPUT ${BT_HOST_EMPTY_SRC_FILES} ${BTCONF_FILE}
  COMMAND ${MAKE_EXECUTABLE} export EXPORT_DIR=${EXPORT_DIR}
  WORKING_DIRECTORY ${GeckoSDK_ROOT_DIR}/app/bluetooth/example_host/bt_host_empty
  COMMENT "Export project files"
)

add_custom_command(
  OUTPUT ${AUTOGEN_DIR}/gatt_db.c
  COMMAND ${Python3_EXECUTABLE} ${BGBUILD} ${BTCONF_DIR} -o ${AUTOGEN_DIR}
  DEPENDS ${BTCONF_FILE}
  COMMENT "Generate GATT database"
)

add_executable(bt_host_empty
  ${BT_HOST_EMPTY_SRC_FILES}
  ${AUTOGEN_DIR}/gatt_db.c
)

add_dependencies(bt_host_empty cpc cpcd)

target_include_directories(bt_host_empty BEFORE PRIVATE
  ${BT_HOST_EMPTY_INCLUDES}
  ${cpcd_SOURCE_DIR}/lib
  ${AUTOGEN_DIR}
)

target_compile_definitions(bt_host_empty PRIVATE
  -DHOST_TOOLCHAIN
  -DPOSIX
  -DCPC
  -DSL_CATALOG_APP_LOG_PRESENT
  -DSL_CATALOG_NCP_GATT_PRESENT
  -DSL_CATALOG_NCP_VERSION_PRESENT
)

set_source_files_properties(
  ${EXPORT_DIR}/app/bluetooth/common_host/host_comm/host_comm_posix.c
  PROPERTIES COMPILE_FLAGS -Wno-stringop-truncation
)

target_link_libraries(bt_host_empty PRIVATE pthread util cpc)
target_link_directories(bt_host_empty PRIVATE ${cpcd_BINARY_DIR})

install(TARGETS bt_host_empty RUNTIME DESTINATION "bin" COMPONENT uic-bt-host-empty)

add_component_to_uic(
  uic-bt-host-empty # Package Name
  "Universal IOT controller - Bluetooth Host Empty Project"
  "${CMAKE_PROJECT_NAME}-bt-host-empty" # Package Debian filename
  "${CMAKE_PROJECT_NAME}-cpcd" # Package Depends on
  ""
  ""
)
