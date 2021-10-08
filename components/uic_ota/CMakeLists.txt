# Library
add_library(
  uic_ota SHARED platform/${COMPATIBLE_PLATFORM}/ota_time.cpp src/ota.cpp
                 src/ota_cache.cpp src/ota_mqtt.cpp src/ota_download.cpp)
target_include_directories(
  uic_ota
  PRIVATE src 
  PUBLIC platform/${COMPATIBLE_PLATFORM} include)

target_link_libraries(uic_ota PUBLIC uic_definitions uic_contiki uic_mqtt
                                     uic_log)

# GCC 8.3 define the std::filesystem as a separate library
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_link_libraries(uic_ota PRIVATE stdc++fs)
endif()

# Mock target_add_mock(uic_ota)

# Test
add_subdirectory(test)
