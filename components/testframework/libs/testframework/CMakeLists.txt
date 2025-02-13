# Prevent multiple "inclusions" of TestFramework
if (NOT COMMAND ADD_UNITY_TEST)
  message(STATUS "TestFramework from ${CMAKE_CURRENT_SOURCE_DIR} included")
  add_definitions( -DUNIT_TEST )

  set(TEST_TOOLS_DIR ${CMAKE_CURRENT_LIST_DIR} CACHE INTERNAL test_tools_dir)

  find_package(Python3 COMPONENTS Interpreter)

  ##
  # @b Syntax
  #
  # &emsp; @c add_unity_test( \b NAME \<name\> [TEST_BASE \<file.\<c|cpp\>\>] FILES \<fileN\> LIBRARIES \<libN\> [USE_CPP])
  #
  # Function for adding a unit test CMake target.
  #
  #
  # @b Example @b 1
  #
  #
  # from @b Components\\FileSystem\\Test\\CMakeLists.txt file
  # @code{.py}
  # add_unity_test(NAME TestFileSystem FILES ../FileSystem.c TestFileSystem.cpp LIBRARIES Assert USE_CPP)
  # @endcode
  #
  #
  # @b Example @b 2
  #
  #
  # from @b ZWave\\Protocol\\Test\\CMakeLists.txt file
  # @code{.py}
  # add_unity_test(NAME TestTransmitSingleCastLinkLayer 
  #                FILES ../ZW_DataLinkLayer.c TestTransmitSingleCastLinkLayer.c 
  #                LIBRARIES mock ZW_RadioPhyMock Assert )
  # @endcode
  #
  #
  # @b Parameters
  # <TABLE cellspacing=0 cellborder=0>
  #   <TR>
  #     <TD>&emsp;&emsp;&emsp;</TD><TD><TT>\[in\]</TT></TD><TD><b>NAME \<name\></b></TD>
  #     <TD>Name of test executeable to build</TD>
  #   </TR>
  #   <TR>
  #     <TD></TD><TD><TT>\[in\]</TT></TD><TD><b>TEST_BASE \<file.\<c|cpp\>\></b></TD>
  #     <TD>\[Optional\] Unit test file from which the runner should be generated, if TEST_BASE is not
  #                      provided, the \<name\>.\<c|cpp\> from first parameter will be used.</TD>
  #   </TR>
  #   <TR>
  #     <TD></TD><TD><TT>\[in\]</TT></TD><TD><b>FILES \<fileN\></b></TD>
  #     <TD>List of files used for building the test</TD>
  #   </TR>
  #   <TR>
  #     <TD></TD><TD><TT>\[in\]</TT></TD><TD><b>LIBRARIES \<libN\></b></TD>
  #     <TD>List of libraries to link for the test executable (unity is automatically included)</TD>
  #   </TR>
  #   <TR>
  #     <TD></TD><TD><TT>\[in\]</TT></TD><TD><b>USE_CPP</b></TD>
  #     <TD>\[Optional\] Set this flag if the test executable must be compiled using C++</TD>
  #   </TR>
  # </TABLE>
  #
  function(ADD_UNITY_TEST)
    set(OPTIONS USE_CPP DISABLED USE_UNITY_WITH_CMOCK)
    set(SINGLE_VALUE_ARGS "NAME" "TEST_BASE")
    set(MULTI_VALUE_ARGS "FILES" "LIBRARIES" "INCLUDES")
    cmake_parse_arguments(ADD_UNITY_TEST "${OPTIONS}" "${SINGLE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" ${ARGN} )

    set(RUNNER_EXTENSION c)
    if (ADD_UNITY_TEST_USE_CPP)
      set(RUNNER_EXTENSION cpp)
    endif (ADD_UNITY_TEST_USE_CPP)

    set(RUNNER_BASE "${ADD_UNITY_TEST_NAME}.${RUNNER_EXTENSION}")
    if (NOT ${ADD_UNITY_TEST_TEST_BASE} STREQUAL "")
      set(RUNNER_BASE "${ADD_UNITY_TEST_TEST_BASE}")
    endif (NOT ${ADD_UNITY_TEST_TEST_BASE} STREQUAL "")

    if ("${ADD_UNITY_TEST_NAME}" STREQUAL "")
      list(GET ADD_UNITY_TEST_UNPARSED_ARGUMENTS 0 ADD_UNITY_TEST_NAME)
      list(REMOVE_AT ADD_UNITY_TEST_UNPARSED_ARGUMENTS 0)
      set(RUNNER_BASE "${ADD_UNITY_TEST_NAME}.${RUNNER_EXTENSION}")
      set(ADD_UNITY_TEST_FILES "${ADD_UNITY_TEST_UNPARSED_ARGUMENTS}")
    endif ("${ADD_UNITY_TEST_NAME}" STREQUAL "")

    add_executable(${ADD_UNITY_TEST_NAME} ${ADD_UNITY_TEST_NAME}_runner.${RUNNER_EXTENSION} ${RUNNER_BASE} ${ADD_UNITY_TEST_FILES})

    add_custom_command(OUTPUT ${ADD_UNITY_TEST_NAME}_runner.${RUNNER_EXTENSION}
        COMMAND ${PYTHON_EXECUTABLE} ${TEST_TOOLS_DIR}/gen_test_runner.py  ${RUNNER_BASE} > ${CMAKE_CURRENT_BINARY_DIR}/${ADD_UNITY_TEST_NAME}_runner.${RUNNER_EXTENSION}
        DEPENDS ${RUNNER_BASE} ${TEST_TOOLS_DIR}/gen_test_runner.py
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    add_test(${ADD_UNITY_TEST_NAME} ${ADD_UNITY_TEST_NAME} )
    target_link_libraries( ${ADD_UNITY_TEST_NAME} ${ADD_UNITY_TEST_LIBRARIES})
    if (NOT ADD_UNITY_TEST_USE_UNITY_WITH_CMOCK)
      target_link_libraries( ${ADD_UNITY_TEST_NAME} unity)
    endif (NOT ADD_UNITY_TEST_USE_UNITY_WITH_CMOCK)

    target_include_directories(${ADD_UNITY_TEST_NAME} PRIVATE . ${ADD_UNITY_TEST_INCLUDES})

    if(${ADD_UNITY_TEST_DISABLED})
      set_tests_properties(${TEST_NAME} PROPERTIES DISABLED True)
    endif()
  endfunction(ADD_UNITY_TEST)

  # compile the unity version bundled along with cmock sources.
  add_library(unity ../cmock/vendor/unity/src/unity.c)
  target_include_directories(unity PUBLIC ../cmock/vendor/unity/src)
  target_compile_options(unity PRIVATE "-fPIC")

  # Build the cmock library and link the above compiled unity with the cmock library
  add_library(cmock2 STATIC ../cmock/src/cmock.c)
  target_include_directories(cmock2 PUBLIC ../cmock/src)
  target_link_libraries(cmock2 PUBLIC unity)
  target_compile_options(cmock2 PRIVATE "-fPIC")

  # Check whether the building of mock framework is disabled.
  # Building of the mock framework can be disabled by chip vendors simply by adding the following line to the specific project at higher level:
  # e.g.: set(DISABLE_MOCK 1)
  # Warning, Do not set DISABLE_MOCK in this file.
  if (NOT DEFINED DISABLE_MOCK)
    add_subdirectory(mock)
  endif (NOT DEFINED DISABLE_MOCK)
else(NOT COMMAND ADD_UNITY_TEST)
  message(STATUS "TestFramework from ${CMAKE_CURRENT_SOURCE_DIR} NOT included")
endif(NOT COMMAND ADD_UNITY_TEST)
