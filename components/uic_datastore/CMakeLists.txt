find_package(SQLite3 3.24 REQUIRED)

# Library
add_library(uic_datastore SHARED
  src/datastore.c
  src/datastore_fixt.c
  src/datastore_internals.c
  src/datastore_attributes.c
)
target_include_directories(
  uic_datastore
  PUBLIC include
         ${CMAKE_BINARY_DIR}/include/
  PRIVATE ${SQLite3_INCLUDE_DIRS})
target_link_libraries(uic_datastore uic_log ${SQLite3_LIBRARIES})

target_compile_definitions(
  uic_datastore PRIVATE CMAKE_PROJECT_VERSION="${CMAKE_PROJECT_VERSION}")
target_add_mock(uic_datastore)

add_subdirectory(test)
