add_definitions(-DBOOST_LOG_DYN_LINK)
set(Boost_USE_MULTITHREADED ON)

find_package(Boost COMPONENTS log log_setup REQUIRED)
add_library(uic_log OBJECT platform/${COMPATIBLE_PLATFORM}/sl_log.cpp)

if(EXISTS ${COMMON_LOCATION})
  target_include_directories(uic_log PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<BUILD_INTERFACE:${COMMON_LOCATION}/include>)
else()
  target_include_directories(uic_log PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../../include>)
endif()

target_link_libraries(
  uic_log
  PUBLIC uic_config
  PRIVATE Boost::log Boost::log_setup pthread)

if(BUILD_TESTING)
  target_add_unittest(uic_log SOURCES test/sl_log_test.c)
endif()

# Disable compiler note for ps abi on Linux
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  target_compile_options(uic_log PRIVATE -Wno-psabi)
endif()
install_headers(uic_log uic libuic-dev)
