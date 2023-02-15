add_library(zigpc_binding 
    src/zigpc_binding_request.cpp
    src/zigpc_binding_response.cpp
    src/zigpc_binding_mqtt_helpers.cpp 
    src/zigpc_binding_mqtt.cpp 
    src/zigpc_binding_setup.cpp)

target_include_directories(
 zigpc_binding 
  PRIVATE src
  PUBLIC include)

target_link_libraries(
 zigpc_binding 
  INTERFACE zigpc_common
  PRIVATE zigpc_datastore zigpc_gateway zigpc_ucl uic_dotdot uic_mqtt uic_log)

if(BUILD_TESTING)
  target_add_mock(zigpc_binding)

  add_subdirectory(test)
endif()

