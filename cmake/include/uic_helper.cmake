set(DIR_OF_UIC_HELPER_CMAKE
    ${CMAKE_CURRENT_LIST_DIR}
    CACHE INTERNAL "DIR_OF_UIC_HELPER_CMAKE")

function(collect_object_files TARGET TARGET_OBJS EXCLUDES)
  set(TARGET_OBJS
      $<TARGET_OBJECTS:${TARGET}>
      PARENT_SCOPE)
  if(NOT "${EXCLUDES}" STREQUAL "")
    list(LENGTH EXCLUDES length)
    math(EXPR list_last "${length} - 1")
    list(GET EXCLUDES ${list_last} last_element)

    foreach(ex ${EXCLUDES})
      string(REPLACE "." "\\." cleaned_ex ${ex})
      string(APPEND regex ".*${cleaned_ex}\\.o.*")
      if(${length} GREATER 1 AND NOT "${last_element}" STREQUAL "${ex}")
        string(APPEND regex "|")
      endif()
    endforeach()

    set(TARGET_OBJS
        $<FILTER:$<REMOVE_DUPLICATES:$<TARGET_OBJECTS:${TARGET}>>,EXCLUDE,${regex}>
        PARENT_SCOPE)
  endif()
endfunction()

# helper tags to pack static libraries as a whole library. meaning all symbols
# are imported.
if(APPLE)
  set(WHOLE_ARCHIVE_START -Wl,-undefined -Wl,dynamic_lookup -Wl,-all_load)
  set(WHOLE_ARCHIVE_END "")
else()
  set(WHOLE_ARCHIVE_START -Wl,--whole-archive)
  set(WHOLE_ARCHIVE_END -Wl,--no-whole-archive)
endif()

# This function is able to collect a bunch of static liberries (.a) and bundle
# them into a shared library. All libraries must be defined cmake targets, ie
# defined using the add_library() command
#
# Usage: uic_add_shared_library( <output_lib> <lib1> <lib2> ... <libN>)
function(uic_add_shared_library)
  # Parse the arguments
  list(POP_FRONT ARGV LIBNAME)
  set(LIBS ${ARGV})
  # Gather a list of genrator expressions which can resolve the achive file
  # belonging to the target
  foreach(L ${LIBS})
    get_target_property(TARGET_TYPE ${L} TYPE)
    if(NOT (${TARGET_TYPE} MATCHES "INTERFACE_LIBRARY"))
      list(APPEND GENERATOR_EXPRS "$<TARGET_OBJECTS:${L}>")
    endif()

    get_target_property(includes ${L} INTERFACE_INCLUDE_DIRECTORIES)
    if(includes)
      list(APPEND target-includes "${includes}")
    endif()

    set_property(TARGET ${L} PROPERTY POSITION_INDEPENDENT_CODE 1)
  endforeach()

  list(REMOVE_DUPLICATES target-includes)

  # We just make an empty C file to keep Cmake happy
  set(DUMMYFILE ${CMAKE_CURRENT_BINARY_DIR}/${LIBNAME}_dummy.cpp)
  file(TOUCH ${DUMMYFILE})
  add_library(${LIBNAME} SHARED ${DUMMYFILE})
  add_dependencies(${LIBNAME} ${LIBS})
  target_include_directories(${LIBNAME} PUBLIC ${target-includes})
  target_link_options(${LIBNAME} PRIVATE ${WHOLE_ARCHIVE_START}
                      ${GENERATOR_EXPRS} ${WHOLE_ARCHIVE_END})
endfunction()

if(NOT TARGET sl_status_strings)
  if(EXISTS ${COMMON_LOCATION})
    add_custom_command(
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/include/sl_status_strings.h
      COMMAND python3 ${COMMON_LOCATION}/scripts/sl_status_strings.py ${COMMON_LOCATION}/include >
              ${CMAKE_CURRENT_BINARY_DIR}/include/sl_status_strings.h
      DEPENDS ${COMMON_LOCATION}/include/sl_status.h
              ${COMMON_LOCATION}/scripts/sl_status_strings.py)
    add_custom_target(
      sl_status_strings
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/include/sl_status_strings.h)
  else()
    add_custom_command(
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/include/sl_status_strings.h
      COMMAND python3 ${DIR_OF_UIC_HELPER_CMAKE}/../../scripts/sl_status_strings.py ${DIR_OF_UIC_HELPER_CMAKE}/../../include >
              ${CMAKE_CURRENT_BINARY_DIR}/include/sl_status_strings.h
      DEPENDS ${DIR_OF_UIC_HELPER_CMAKE}/../../include/sl_status.h
              ${DIR_OF_UIC_HELPER_CMAKE}/../../scripts/sl_status_strings.py)
    add_custom_target(
      sl_status_strings
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/include/sl_status_strings.h)
  endif()
endif()
