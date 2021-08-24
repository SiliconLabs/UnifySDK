# Unit Tests We need to compile src files with executable as command classes is
# intialized at compile time
file(GLOB COMMANDS_SRC ../../dev_commands/src/*.cpp)
file(GLOB COMMANDS_LIB ../../dev_commands/lib/*.c*)
file(GLOB DEV_RUN_CLI_SRC ../src/*.c*)

set(Boost_USE_MULTITHREADED ON)
find_package(Boost COMPONENTS system)

# Create lib for testing
add_library(dev_generic_command_handler ${DEV_RUN_CLI_SRC} ${COMMANDS_LIB}
                                        ${COMMANDS_SRC} dummy_class.cpp)

# Include folders
target_include_directories(
  dev_generic_command_handler
  PUBLIC ${Boost_INCLUDE_DIRS}
  PUBLIC ../../dev_commands/lib ../include ../src)

target_link_libraries(dev_generic_command_handler dev_cli_config uic_contiki
                      uic_dotdot_mqtt Boost::system)

# Test the lib
target_add_unittest(dev_generic_command_handler SOURCES dev_generic_command_handler.c)
