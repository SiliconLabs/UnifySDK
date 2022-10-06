# Library
add_library(uic_gbl_interface src/uic_gbl_interface.cpp)
target_include_directories(uic_gbl_interface PUBLIC include)
target_link_libraries(uic_gbl_interface uic_log uic_contiki_platform)

# Testing
target_add_unittest(uic_gbl_interface SOURCES test/uic_gbl_interface_test.c)

# Mock
target_add_mock(uic_gbl_interface)
install_headers(uic_gbl_interface "uic" "uic-dev")
