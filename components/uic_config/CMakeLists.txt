# Dependencies
set(Boost_USE_MULTITHREADED ON)
find_package(Boost COMPONENTS program_options)
find_package(Yaml-cpp REQUIRED)

if(NOT YAML_CPP_LIBRARIES)
  # On most machines the yaml-cpp configuration doesnt set YAML_CPP_LIBRARIES,
  # YAML_CPP_FOUND and YAML_CPP_INCL_DIRS even though it seems that a find
  # package config is found and some environment is set. explicitly set
  # '-lyaml-cpp' so that we can fallback on PKG-CONFIG to resolve our dependency
  set(YAML_CPP_LIBRARIES "yaml-cpp")
endif()

# Library
set(CONFIG_SRC platform/${COMPATIBLE_PLATFORM}/config.cpp)
set(CONFIG_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/include/)
set(CONFIG_DEPS Boost::program_options ${YAML_CPP_LIBRARIES})
add_library(uic_config ${CONFIG_SRC})
target_include_directories(
  uic_config
  PUBLIC $<BUILD_INTERFACE:${CONFIG_INCLUDE}>
  PRIVATE platform/${COMPATIBLE_PLATFORM})
target_link_libraries(uic_config ${CONFIG_DEPS})
target_compile_definitions(
  uic_config PRIVATE CMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}")
install_headers(uic_config uic libuic-dev)

if(BUILD_TESTING)
  add_subdirectory(test)

  target_add_mock(uic_config)
endif()
