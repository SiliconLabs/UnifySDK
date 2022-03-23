add_library(
  unify_validator SHARED
  src/unify_validator.cpp)

target_include_directories(unify_validator PUBLIC include)

add_subdirectory(test)
