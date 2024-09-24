add_library(zwave_command_class_test_helpers 
    zwave_command_class_test_helper.cpp
    zwave_resolver_function_helper.cpp
    zwave_raw_frame_test_helper.c
    )

target_include_directories(zwave_command_class_test_helpers 
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(zwave_command_class_test_helpers 
                    # Helpers
                    zpc_attribute_store_test_helper
                    # Mock
                    zwave_command_handler_mock
                    uic_attribute_resolver_mock)