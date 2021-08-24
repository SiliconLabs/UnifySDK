add_definitions(-DBOOST_LOG_DYN_LINK)
set(Boost_USE_MULTITHREADED ON)

find_package(Boost COMPONENTS log log_setup)
set(LOG_SRC platform/${COMPATIBLE_PLATFORM}/sl_log.cpp)
add_library(uic_log SHARED ${LOG_SRC})
target_include_directories(uic_log PUBLIC include)
target_link_libraries(
  uic_log
  PUBLIC uic_config
  PRIVATE Boost::log Boost::log_setup)

target_add_unittest(uic_log SOURCES test/sl_log_test.c)
# Disable compiler note for ps abi on Linux
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  target_compile_options(uic_log PRIVATE -Wno-psabi)
endif()
