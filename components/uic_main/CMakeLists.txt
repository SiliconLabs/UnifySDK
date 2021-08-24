set(UIC_MAIN_SRC
    src/uic_main.c
    src/uic_init.c
    src/uic_main_loop.c
    src/uic_component_fixtures_array.c
    src/uic_component_fixtures.c
    src/uic_signal_handler.c
    platform/${COMPATIBLE_PLATFORM}/uic_main_externals_platform.c)
set(UIC_MAIN_INCLUDE include/)

add_library(uic_main SHARED ${UIC_MAIN_SRC})
target_link_libraries(uic_main uic_mqtt uic_stdin uic_contiki_platform)
target_include_directories(
  uic_main
  PUBLIC ${UIC_MAIN_INCLUDE}
         ${CMAKE_BINARY_DIR}/include/
  PRIVATE src/ platform/${COMPATIBLE_PLATFORM}/)

# CMock
add_mock(uic_main_mock include/uic_component_fixtures.h include/uic_main.h)
target_interface_libraries(uic_main_mock uic_contiki)

target_add_unittest(uic_main
SOURCES test/test_uic_main.c
EXCLUDE uic_component_fixtures_array.c)
