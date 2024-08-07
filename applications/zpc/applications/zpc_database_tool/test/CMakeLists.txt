if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # "--allow-multiple-definition" can cause some weird symptons as we leave it
  # up to the linker which symbol gets merged into the test binary.
  add_link_options("-Wl,--unresolved-symbols=ignore-in-object-files"
                   "-Wl,--allow-multiple-definition")
endif()

target_add_unittest(
  zpc_database_utils
  NAME
  zpc_database_upgrade_tool_test
  SOURCES
  zpc_database_upgrade_tool_test.cpp
)

# Copy asset in test folder
execute_process(COMMAND ${CMAKE_COMMAND} 
    -E copy_if_different 
    ${CMAKE_CURRENT_SOURCE_DIR}/assets/v2.db
    ${CMAKE_CURRENT_BINARY_DIR}/v2.db)