# Find nlohmann_json, unfortunately the find_package doesn't work for this when
# cross compiling, thus we use the simplified method find_path instead
find_path(nlohmann_json_include nlohmann/json.hpp REQUIRED)

add_library(uic_smartstart_management_mock smartstart_management_mock.cpp)

target_include_directories(
  uic_smartstart_management_mock
  PUBLIC . ../include
  PRIVATE ${nlohmann_json_include})

# Assuming that the contiki stub will always be used  with the mock here
target_link_libraries(uic_smartstart_management_mock PRIVATE uic_contiki_stub
                                                             uic_log)
