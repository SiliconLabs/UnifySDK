add_executable(cpc-hci-bridge cpc-hci-bridge.c)

target_link_libraries(cpc-hci-bridge
  cpc
  uic_log
  util
  pthread
)

get_target_property(CPC_SOURCE_DIR cpc SOURCE_DIR)
target_include_directories(cpc-hci-bridge PRIVATE ${CPC_SOURCE_DIR}/lib)

install(TARGETS cpc-hci-bridge RUNTIME DESTINATION "bin" COMPONENT uic-cpc-hci-bridge)
install(DIRECTORY DESTINATION /var/lib/cpc-hci-bridge COMPONENT uic-cpc-hci-bridge)

if(NOT APPLE)
  install(
    FILES debconf/uic-cpc-hci-bridge.service
    DESTINATION /lib/systemd/system/
    COMPONENT uic-cpc-hci-bridge)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/postinst
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/postinst)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/prerm
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/prerm)

add_component_to_uic(
  uic-cpc-hci-bridge
  "Universal IOT controller - cpc-hci-bridge"
  "${CMAKE_PROJECT_NAME}-cpc-hci-bridge"
  "libuic"
  "${CMAKE_BINARY_DIR}/applications/aox/applications/cpc-hci-bridge/debconf/postinst;\
  ${CMAKE_BINARY_DIR}/applications/aox/applications/cpc-hci-bridge/debconf/prerm")
