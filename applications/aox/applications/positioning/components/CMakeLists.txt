add_subdirectory(positioning_application_monitoring)
add_subdirectory(positioning_id)

# Components from GSDK

# ##############################################################################
# Angle queue component
# ##############################################################################
add_library(angle_queue
  ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/angle_queue/angle_queue.c
)
target_include_directories(angle_queue
  PUBLIC ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/angle_queue
)
target_link_libraries(angle_queue aoa_util)

# ##############################################################################
# Location calculation engine
# ##############################################################################
add_library(loc
  ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/aoa_loc/aoa_loc.c
)
target_include_directories(loc
  PUBLIC ${GeckoSDK_ROOT_DIR}/app/bluetooth/common_host/aoa_loc
  PRIVATE ${GeckoSDK_ROOT_DIR}/platform/common/inc
)
target_link_libraries(loc rtl_lib aoa_util)

target_compile_options(loc
                       PUBLIC
                       -Wno-maybe-uninitialized)
