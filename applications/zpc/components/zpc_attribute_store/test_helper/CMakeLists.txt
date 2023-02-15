add_library(zpc_attribute_store_test_helper zpc_attribute_store_test_helper.cpp)
target_include_directories(zpc_attribute_store_test_helper PUBLIC .)
target_link_libraries(zpc_attribute_store_test_helper
                      PUBLIC zpc_attribute_store )
