add_library(uic_multi_invoke INTERFACE)
target_include_directories(uic_multi_invoke INTERFACE include)

install_headers(uic_multi_invoke "uic" "uic-dev")
add_subdirectory(test)
