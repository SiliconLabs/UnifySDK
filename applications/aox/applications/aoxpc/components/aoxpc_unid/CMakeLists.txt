add_library(aoxpc_unid
  src/aoxpc_unid.c)

target_include_directories(
  aoxpc_unid
  PUBLIC include
  PRIVATE src)

target_link_libraries(
  aoxpc_unid
  PUBLIC aoa_utils
  PRIVATE uic_log)

# Mocks
target_add_mock(aoxpc_unid)

add_subdirectory(test)
