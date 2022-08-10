# Find nlohmann_json, unfortunately the find_package doesn't work for this when
# cross compiling, thus we use the simplified method find_path instead
find_path(nlohmann_json_include nlohmann/json.hpp REQUIRED)

add_library(uic_smartstart_management SHARED
    src/smartstart_management.cpp
)

target_include_directories(uic_smartstart_management PUBLIC
  include ${nlohmann_json_include}
)

target_link_libraries(uic_smartstart_management PUBLIC
  uic_contiki
  uic_mqtt
  uic_log
)

add_subdirectory(mocks)
add_subdirectory(test)