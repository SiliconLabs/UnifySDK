# Library
run_zap(zap/gen-templates.json)

add_library(
  uic_dotdot_attribute_store
  ${ZAP_CURRENT_OUTPUT_DIR}/src/uic_dotdot_attribute_store_registration.cpp)

target_include_directories(
  uic_dotdot_attribute_store
  PRIVATE src ${ZAP_CURRENT_OUTPUT_DIR}/src
  PUBLIC include ${ZAP_CURRENT_OUTPUT_DIR}/include)

target_link_libraries(
  uic_dotdot_attribute_store
  PUBLIC uic_definitions uic_dotdot
  PRIVATE uic_attribute_store)
install_headers(uic_dotdot_attribute_store "uic" "uic-dev")
