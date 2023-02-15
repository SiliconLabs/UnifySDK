# CMock
add_mock(uic_main_mock ../include/uic_component_fixtures.h ../include/uic_main.h)
target_interface_libraries(uic_main_mock uic_contiki)
target_include_directories(uic_main_mock PRIVATE ../include)

add_library(uic_main_shared SHARED $<TARGET_OBJECTS:uic_main>) 
target_link_libraries(uic_main_shared unify)

target_add_unittest(uic_main_shared
NAME uic_main_test
SOURCES test_uic_main.c
EXCLUDE uic_component_fixtures_array.c)

target_include_directories(uic_main_test PUBLIC
  ../src
)

