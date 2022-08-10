# Library
find_program(ZAP_EXECUTABLE "zap" REQUIRED)
run_zap(zap/gen-templates.json)

add_library(
  uic_dotdot_attribute_store SHARED
  ${CMAKE_CURRENT_BINARY_DIR}/src/uic_dotdot_attribute_store_registration.cpp
)

target_include_directories(
  uic_dotdot_attribute_store
  PRIVATE src ${CMAKE_CURRENT_BINARY_DIR}/src
  PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/include
)

target_link_libraries(uic_dotdot_attribute_store
  PUBLIC
    uic_definitions
    uic_dotdot
  PRIVATE
  uic_attribute_store
)