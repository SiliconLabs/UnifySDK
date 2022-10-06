add_library(unify_validator src/unify_validator.cpp)
target_include_directories(unify_validator PUBLIC include)

install_headers(unify_validator "uic" "uic-dev")
add_subdirectory(test)
