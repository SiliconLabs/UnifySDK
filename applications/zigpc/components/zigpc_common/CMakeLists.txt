# Source/Include
set(zigpc_common_SRC src/zigpc_common_zigbee.c src/zigpc_common_observable.c
                     src/zigpc_common_zcl_frame.c)

# Lib
add_library(zigpc_common STATIC ${zigpc_common_SRC})
target_include_directories(
  zigpc_common
  PUBLIC include
  PRIVATE src)

target_link_libraries(zigpc_common PRIVATE uic_log)

# Testing
add_subdirectory(test)

# Mocks
target_add_mock(zigpc_common)
