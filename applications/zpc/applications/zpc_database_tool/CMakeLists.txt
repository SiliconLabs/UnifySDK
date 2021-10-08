# Migration tool
add_executable(zpc_database_tool zpc_database_tool.cpp)
target_link_libraries(zpc_database_tool zpc_attribute_store uic_config)

install(TARGETS zpc_database_tool RUNTIME DESTINATION "bin" COMPONENT uic-zpc)

# # Upgrade tool
# add_executable(zpc_database_upgrade_tool zpc_database_upgrade_tool.cpp
#   src/zpc_database_helper.cpp
# )
# target_include_directories(zpc_database_upgrade_tool PUBLIC include)
# target_link_libraries(zpc_database_upgrade_tool zpc_attribute_store zpc_datastore uic_config)

# install(TARGETS zpc_database_upgrade_tool RUNTIME DESTINATION "bin" COMPONENT uic-zpc)

# Recover tool
add_executable(zpc_database_recover_tool zpc_database_recover_tool.cpp
  src/zpc_database_helper.cpp
)
target_include_directories(zpc_database_recover_tool PUBLIC include)
target_link_libraries(zpc_database_recover_tool zpc_attribute_store zpc_datastore uic_config)

install(TARGETS zpc_database_recover_tool RUNTIME DESTINATION "bin" COMPONENT uic-zpc)