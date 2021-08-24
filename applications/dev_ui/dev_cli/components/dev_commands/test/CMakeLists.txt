# Unit Tests We need to compile src files with executable as command classes is
# intialized at compile time
file(GLOB COMMANDS_LIB ../src/*.c*)
file(GLOB DEV_RUN_CLI_LIB ../../dev_run_cli/src/*.c*)

set(Boost_USE_MULTITHREADED ON)
find_package(Boost COMPONENTS system)

# Create lib for testing
add_library(command_smartstartlist_lib ${DEV_RUN_CLI_LIB} ${COMMANDS_LIB})
target_link_libraries(command_smartstartlist_lib uic_dotdot_mqtt uic_contiki
                      dev_cli_config Boost::system)

target_link_libraries(command_smartstartlist_lib uic_dotdot_mqtt uic_contiki
                      dev_cli_config Boost::system)
target_include_directories(command_smartstartlist_lib
                           PRIVATE . ../lib ../../dev_run_cli/include)

# Test the lib
target_add_unittest(command_smartstartlist_lib SOURCES dev_commands_lib.c)
