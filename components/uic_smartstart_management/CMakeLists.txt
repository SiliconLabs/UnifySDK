add_library(uic_smartstart_management SHARED
    src/smartstart_management.cpp
)

target_include_directories(uic_smartstart_management PUBLIC include)
target_link_libraries(uic_smartstart_management PUBLIC
  uic_contiki
  uic_mqtt
  uic_log
)

add_subdirectory(test)