find_package(SQLite3 3.24 REQUIRED)

# Library
add_library(uic_datastore src/datastore.c src/datastore_attributes.c
                          src/datastore_fixt.c src/datastore_internals.c)
target_include_directories(
  uic_datastore
  PUBLIC include
  PRIVATE ${SQLite3_INCLUDE_DIRS})
target_link_libraries(
  uic_datastore
  PUBLIC uic_log uic_config
  PRIVATE ${SQLite3_LIBRARIES} unify_version)

install_headers(uic_datastore "uic" "uic-dev")

target_compile_definitions(
  uic_datastore PRIVATE CMAKE_PROJECT_VERSION="${CMAKE_PROJECT_VERSION}")
target_add_mock(uic_datastore)

add_subdirectory(test)
