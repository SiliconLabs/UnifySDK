add_executable(zpc_database_tool zpc_database_tool.cpp)
target_link_libraries(zpc_database_tool zpc_attribute_store uic_config)

install(TARGETS zpc_database_tool RUNTIME DESTINATION "bin" COMPONENT uic-zpc)
