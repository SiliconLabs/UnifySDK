# Library
add_library(uic_definitions INTERFACE)
target_include_directories(uic_definitions INTERFACE include)
install_headers(uic_definitions "uic" "uic-dev")
