if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # "--allow-multiple-definition" can cause some weird symptons as we leave it
  # up to the linker which symbol gets merged into the test binary.
  add_link_options("-Wl,--unresolved-symbols=ignore-in-object-files"
                   "-Wl,--allow-multiple-definition")
endif()
target_add_unittest(
  zpc_dotdot_mqtt
  SOURCES
  zpc_dotdot_mqtt_group_dispatch_test.c
  DEPENDS
  unify_dotdot_attribute_store
  zpc_attribute_store_mock
  uic_attribute_store_mock
  uic_dotdot_mqtt_mock
  zcl_cluster_servers_mock)
