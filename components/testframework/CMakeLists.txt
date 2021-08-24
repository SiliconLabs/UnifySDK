set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED)

add_subdirectory(libs/testframework)

add_library(unity2 libs/cmock/vendor/unity/src/unity.c src/unity_helpers.cpp)
target_include_directories(
  unity2
  PUBLIC libs/cmock/vendor/unity/src include PRIVATE ${Boost_INCLUDE_DIRS})
target_compile_definitions(unity2 PUBLIC UNITY_SUPPORT_64 UNITY_INCLUDE_DOUBLE
                                         UNITY_DOUBLE_PRECISION=1e-12f)
disable_target_check(unity2)
target_compile_options(unity2 PRIVATE "-fPIC")

add_library(cmock STATIC libs/cmock/src/cmock.c)
target_include_directories(cmock PUBLIC libs/cmock/src)
target_link_libraries(cmock unity2)
target_compile_options(cmock PRIVATE "-fPIC")

include(add_mock.cmake)
include(target_add_unittest.cmake)

add_library(uic_contiki_stub SHARED src/contiki_test_helper.c $<TARGET_OBJECTS:uic_contiki>)
target_include_directories(uic_contiki_stub PUBLIC include $<TARGET_PROPERTY:uic_contiki,INCLUDE_DIRECTORIES>)
target_link_libraries(uic_contiki_stub $<TARGET_PROPERTY:uic_contiki,LINK_LIBRARIES>)

add_library(uic_mqtt_stub SHARED src/mqtt_mock_helper.cpp $<TARGET_OBJECTS:uic_mqtt_mock>)
target_link_libraries(uic_mqtt_stub PUBLIC $<TARGET_PROPERTY:uic_mqtt_mock,LINK_LIBRARIES>)
target_include_directories(uic_mqtt_stub PUBLIC include $<TARGET_PROPERTY:uic_mqtt_mock,INCLUDE_DIRECTORIES>)
