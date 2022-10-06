# Library
set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED)

run_zap(zap/gen-templates.json)
add_library(uic_dotdot
            ${ZAP_CURRENT_OUTPUT_DIR}/src/dotdot_cluster_id_definitions.c)
target_link_libraries(uic_dotdot uic_definitions)
target_include_directories(uic_dotdot PUBLIC ${ZAP_CURRENT_OUTPUT_DIR}/include/)
install_headers(uic_dotdot "uic" "uic-dev")