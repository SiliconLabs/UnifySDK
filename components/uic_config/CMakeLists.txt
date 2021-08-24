# Dependencies
set(Boost_USE_MULTITHREADED ON)
find_package(Boost COMPONENTS program_options)
find_package(Yaml-cpp REQUIRED)

# Library
set(CONFIG_SRC platform/${COMPATIBLE_PLATFORM}/config.cpp)
set(CONFIG_INCLUDE include/)
set(CONFIG_DEPS Boost::program_options ${YAML_CPP_LIBRARIES})
add_library(uic_config SHARED ${CONFIG_SRC})
target_include_directories(
  uic_config
  PUBLIC ${CONFIG_INCLUDE}
  PRIVATE platform/${COMPATIBLE_PLATFORM})
target_link_libraries(uic_config ${CONFIG_DEPS})
target_compile_definitions(
  uic_config PRIVATE CMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}")

# Test
target_add_unittest(uic_config
SOURCES test/config_test.c)

target_compile_definitions(
  uic_config_test PRIVATE CMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}"
                      CONFIG_UNITTEST)


add_subdirectory(test)

target_add_mock(uic_config)
