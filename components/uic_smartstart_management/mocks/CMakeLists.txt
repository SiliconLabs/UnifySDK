find_package(Boost REQUIRED)

add_library(uic_smartstart_management_mock smartstart_management_mock.cpp)

target_include_directories(uic_smartstart_management PUBLIC
  .
  ../include
  ${Boost_INCLUDE_DIRS}
)

target_link_libraries(uic_smartstart_management_mock
  PRIVATE
    uic_contiki_platform
    uic_smartstart_management
)

