# Source/Include
set(zpc_datastore_SRC src/zpc_datastore.c )
set(zpc_datastore_INCLUDE include/ PUBLIC)
# Lib
add_library(zpc_datastore ${zpc_datastore_SRC})
target_link_libraries(zpc_datastore uic_log uic_datastore)
target_include_directories(zpc_datastore PUBLIC ${zpc_datastore_INCLUDE})
target_compile_definitions(
  zpc_config PRIVATE CMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}" CMAKE_PROJECT_VERSION="${CMAKE_PROJECT_VERSION}")

# mocks
target_add_mock(zpc_datastore)
