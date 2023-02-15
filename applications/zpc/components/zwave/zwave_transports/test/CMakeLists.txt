if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # "--allow-multiple-definition" can cause some weird symptons as we leave it
  # up to the linker which symbol gets merged into the test binary.
  add_link_options("-Wl,--unresolved-symbols=ignore-in-object-files"
                   "-Wl,--allow-multiple-definition")
endif()
# Z-Wave transport fixt test
target_add_unittest(zwave_transports SOURCES zwave_transports_fixt_test.c
                    DEPENDS zwave_s0_mock)
