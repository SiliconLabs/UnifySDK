# Migration tool
add_executable(zpc_database_tool zpc_database_tool.cpp)
target_link_libraries(zpc_database_tool zpc_attribute_store unify)

# Find nlohmann_json, unfortunately the find_package doesn't work for this when
# cross compiling, thus we use the simplified method find_path instead
find_path(nlohmann_json_include nlohmann/json.hpp REQUIRED)

install(TARGETS zpc_database_tool RUNTIME DESTINATION "bin" COMPONENT uic-zpc)

# Library that we can test
add_library(zpc_database_utils STATIC 
  src/zpc_database_helper.cpp
  src/zpc_database_updater.cpp  
)
target_include_directories(zpc_database_utils PUBLIC include)
target_link_libraries(zpc_database_utils zpc_attribute_store zpc_datastore uic_config)


# Upgrade tool
add_executable(zpc_database_upgrade_tool zpc_database_upgrade_tool.cpp)
target_link_libraries(zpc_database_upgrade_tool zpc_database_utils)

install(TARGETS zpc_database_upgrade_tool RUNTIME DESTINATION "bin" COMPONENT uic-zpc)

# Recover tool
add_executable(zpc_database_recover_tool zpc_database_recover_tool.cpp
  src/zpc_database_helper.cpp
)
target_include_directories(zpc_database_recover_tool PRIVATE ${nlohmann_json_include})
target_link_libraries(zpc_database_recover_tool zpc_database_utils)

install(TARGETS zpc_database_recover_tool RUNTIME DESTINATION "bin" COMPONENT uic-zpc)


if(BUILD_TESTING)
  add_subdirectory(test)
endif()