add_library(transport_service
  transport_service.c
)
target_include_directories(transport_service
    PUBLIC include
)
target_link_libraries(transport_service
    PUBLIC uic_contiki
           zpc_utils
)
target_compile_options(transport_service PRIVATE -Wno-unused-variable)
add_subdirectory(test)
target_add_mock(transport_service)
