add_library(
  zigpc_ncp_updater src/zigpc_ncp_updater.cpp)

  target_include_directories(
  zigpc_ncp_updater
  PUBLIC include
  PRIVATE src)

  target_link_libraries(
    zigpc_ncp_updater  
    PRIVATE 
    zigpc_gateway 
    zigpc_common
    zigpc_config
    uic_gbl_interface  
    uic_contiki 
    uic_log)
