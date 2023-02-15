find_package(SQLite3 3.24 REQUIRED)

# Library
add_library(uic_datastore OBJECT src/datastore.c src/datastore_attributes.c
                                 src/datastore_fixt.c src/datastore_internals.c)
target_include_directories(
  uic_datastore
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
  PRIVATE ${SQLite3_INCLUDE_DIRS})
target_link_libraries(
  uic_datastore
  PUBLIC uic_log uic_config
  PRIVATE ${SQLite3_LIBRARIES} unify_version)

install_headers(uic_datastore uic libuic-dev)

target_compile_definitions(
  uic_datastore PRIVATE CMAKE_PROJECT_VERSION="${CMAKE_PROJECT_VERSION}")

if(BUILD_TESTING)
  target_add_mock(uic_datastore)

  add_subdirectory(test)
endif()
