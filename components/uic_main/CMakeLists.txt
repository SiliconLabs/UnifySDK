if(ZWAVE_BUILD_SYSTEM EQUAL 1)
    set(UIC_MAIN_SRC
        src/uic_main_loop.c
        platform/${COMPATIBLE_PLATFORM}/uic_main_externals_platform.c)
    set(UIC_MAIN_INCLUDE
      include/ 
      ${COMPONENTS_DIR}/SwTimer 
      ${ZWAVE_DIR}/Freertos 
      ${SUBTREE_UIC}/components/uic_main_fd/include)
    add_library(uic_main STATIC ${UIC_MAIN_SRC})
    target_link_libraries(uic_main uic_log uic_contiki)
else()
    set(UIC_MAIN_SRC
        src/uic_main.c
        src/uic_init.c
        src/uic_main_loop.c
        src/uic_component_fixtures_array.c
        src/uic_component_fixtures.c
        src/uic_signal_handler.c
        platform/${COMPATIBLE_PLATFORM}/uic_main_externals_platform.c)
    set(UIC_MAIN_INCLUDE include/ ${CURRENT_LIST_DIR}/../../include)
    add_library(uic_main ${UIC_MAIN_SRC})
    target_link_libraries(uic_main uic_mqtt uic_stdin uic_contiki_platform)
endif()

target_include_directories(
  uic_main
  PUBLIC ${UIC_MAIN_INCLUDE}
         ${CMAKE_BINARY_DIR}/include/
  PRIVATE src/ platform/${COMPATIBLE_PLATFORM}/)

install_headers(uic_main "uic" "uic-dev")
# CMock
add_mock(uic_main_mock include/uic_component_fixtures.h include/uic_main.h)
target_interface_libraries(uic_main_mock uic_contiki)
target_include_directories(uic_main_mock PRIVATE ../../include)

target_add_unittest(uic_main
SOURCES test/test_uic_main.c
EXCLUDE uic_component_fixtures_array.c)
