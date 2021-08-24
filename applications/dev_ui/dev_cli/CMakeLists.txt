# We need to compile src files with executable as command classes is
# intialized at compile time

# Adding a new command class to the dev_cli is done by adding the source file
# below
set(COMMAND_CLASSES
  components/dev_commands/src/dev_cli_ui_help.cpp
  components/dev_commands/src/add_smartstart_entry.cpp
  components/dev_commands/src/get_smart_start_list.cpp
  components/dev_commands/src/remove_smartstart_entry.cpp
)

set(COMMAND_FRAMEWORK_SOURCE_FILES
  components/dev_run_cli/src/dev_cli_command_handler.cpp
  components/dev_run_cli/src/dev_cli_command_runner.cpp
  components/dev_run_cli/src/dev_cli_fixt.c
  components/dev_run_cli/src/dev_cli_run.c
)

set(Boost_USE_MULTITHREADED ON)
find_package(Boost COMPONENTS system)

add_subdirectory(components)
add_executable(dev_cli main.c ${COMMAND_FRAMEWORK_SOURCE_FILES} ${COMMAND_CLASSES})

# Include folders
target_include_directories(dev_cli
  PUBLIC ${Boost_INCLUDE_DIRS}
  PUBLIC components/dev_commands/lib components/dev_run_cli/include
  )

target_link_libraries(
  dev_cli
  dev_cli_config
  uic_main
  Boost::system
  )

target_compile_definitions(
  dev_cli PRIVATE CMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}" CMAKE_PROJECT_VERSION="${CMAKE_PROJECT_VERSION}")

#install(TARGETS dev_cli RUNTIME DESTINATION bin)
install(DIRECTORY DESTINATION ${CMAKE_INSTALL_PREFIX}/var/lib/dev_cli
        COMPONENT uic-dev_cli)
install(TARGETS dev_cli RUNTIME DESTINATION "bin" COMPONENT uic-dev_cli)
cpack_add_component(dev_cli
                    DISPLAY_NAME "dev_cli"
                    DESCRIPTION "Universal IOT Controler - Development Client"
                    INSTALL_TYPES Full
                   )

