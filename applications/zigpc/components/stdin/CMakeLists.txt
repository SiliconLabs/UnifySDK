find_package(Edit REQUIRED)

add_library(zigpc_stdin src/zigpc_stdin_commands.cpp)


option(ZIGPC_SL_CLI_DEBUG "Enable Zigpc CLI to call the GeckoSDK CLI via the slcli command" OFF)
if(ZIGPC_SL_CLI_DEBUG)
target_compile_definitions(
  zigpc_stdin PRIVATE ZIGPC_SLI_CLI_ENABLED)
message("Warning : EmberCLI has been enabled. This bypass the UIC logic model and should only be used to debug")
endif()
#We dont want the option to be retained in the cache
unset(ZIGPC_SL_CLI_DEBUG CACHE)


  
target_include_directories(
  zigpc_stdin
  PUBLIC include
  PRIVATE src)

target_link_libraries(zigpc_stdin PRIVATE zigpc_gateway zigpc_datastore
                                          unify)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
