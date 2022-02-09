# Angle queue component

add_library(angle_queue src/angle_queue.c)
target_include_directories(angle_queue PUBLIC include)
target_link_libraries(angle_queue aoa_types aoa_utils uic_log)
