# Library
set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED)

run_zap(zap/gen-templates.json)
add_library(
  uic_dotdot OBJECT
  ${ZAP_CURRENT_OUTPUT_DIR}/src/dotdot_cluster_id_definitions.c
  ${ZAP_CURRENT_OUTPUT_DIR}/src/dotdot_attribute_id_definitions.c)

target_link_libraries(uic_dotdot uic_definitions)
target_include_directories(
  uic_dotdot PUBLIC $<BUILD_INTERFACE:${ZAP_CURRENT_OUTPUT_DIR}/include>)
install_headers(uic_dotdot uic libuic-dev)
# Install UCL xml files
install(
  DIRECTORY dotdot-xml/
  DESTINATION share/uic/ucl
  COMPONENT libuic-dev)
install(
  DIRECTORY shared/
  DESTINATION share/uic/shared
  COMPONENT libuic-dev)
