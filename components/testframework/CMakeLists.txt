set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED)
# Find nlohmann_json, unfortunately the find_package doesn't work for this when
# cross compiling, thus we use the simplified method find_path instead
find_path(nlohmann_json_include nlohmann/json.hpp REQUIRED)

if(EXISTS ${ZW-TESTFRAMEWORK_LOCATION})
  message(STATUS "Found zwave testframework: ${ZW-TESTFRAMEWORK_LOCATION}")
  include_directories(${ZW-TESTFRAMEWORK_LOCATION})
else()
  add_subdirectory(libs/testframework)
endif()

set(DEFAULT_THS-CMOCK_PATH libs/cmock)
if(EXISTS ${THS-CMOCK_LOCATION})
  set(THS-CMOCK_PATH ${THS-CMOCK_LOCATION})
else()
  set(THS-CMOCK_PATH ${DEFAULT_THS-CMOCK_PATH})
endif()
if(EXISTS ${THS-CMOCK_PATH})
  message(STATUS "Found ths-cmock: ${THS-CMOCK_PATH}")
else()
  message(STATUS "Did not find ths-cmock at ${THS-CMOCK_PATH}")
endif()

set(DEFAULT_THS-UNITY_PATH libs/cmock/vendor/unity)
if(EXISTS ${THS-UNITY_LOCATION})
  set(THS-UNITY_PATH ${THS-UNITY_LOCATION})
else()
  set(THS-UNITY_PATH ${DEFAULT_THS-UNITY_PATH})
endif()
if(EXISTS ${THS-UNITY_PATH})
  message(STATUS "Found ths-unity: ${THS-UNITY_PATH}")
else()
  message(STATUS "Did not find ths-unity at ${THS-UNITY_PATH}")
endif()
add_library(unity2 ${THS-UNITY_PATH}/src/unity.c src/unity_helpers.cpp)

target_include_directories(unity2 PUBLIC ${THS-UNITY_PATH}/src include
                                         ${nlohmann_json_include})
target_compile_definitions(unity2 PUBLIC UNITY_SUPPORT_64 UNITY_INCLUDE_DOUBLE
                                         UNITY_DOUBLE_PRECISION=1e-12f)

add_library(cmock STATIC ${THS-CMOCK_PATH}/src/cmock.c)
target_include_directories(cmock PUBLIC ${THS-CMOCK_PATH}/src)
target_link_libraries(cmock unity2)

include(add_mock.cmake)
include(target_add_unittest.cmake)

