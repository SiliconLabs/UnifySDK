add_library(zpc_dotdot_mqtt src/zpc_dotdot_mqtt.c
                            src/zpc_dotdot_mqtt_group_dispatch.cpp)

target_include_directories(
  zpc_dotdot_mqtt
  PUBLIC include ${ZAP_CURRENT_OUTPUT_DIR}/include/
  PRIVATE src)

target_link_libraries(
  zpc_dotdot_mqtt
  PRIVATE unify
          zpc_attribute_store
          zcl_cluster_servers
          )

if(BUILD_TESTING)
  # Unit tests
  # add_subdirectory(test)

  # Mocks
  target_add_mock(zpc_dotdot_mqtt)
endif()
