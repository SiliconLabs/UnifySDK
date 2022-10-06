set(DIR_OF_UIC_HELPER_CMAKE ${CMAKE_CURRENT_LIST_DIR} CACHE INTERNAL "DIR_OF_UIC_HELPER_CMAKE")  

function(collect_object_files TARGET TARGET_OBJS EXCLUDES)
  set(TARGET_OBJS
      $<TARGET_OBJECTS:${TARGET}>
      PARENT_SCOPE)
  if(NOT "${EXCLUDES}" STREQUAL "")
    list(LENGTH EXCLUDES length)
    math(EXPR list_last "${length} - 1")
    list(GET EXCLUDES ${list_last} last_element)

    foreach(ex ${EXCLUDES})
      string(APPEND regex ".*${ex}\.o.*")
      if(${length} GREATER 1 AND NOT "${last_element}" STREQUAL "${ex}")
        string(APPEND regex "|")
      endif()
    endforeach()

    set(TARGET_OBJS
        $<FILTER:$<REMOVE_DUPLICATES:$<TARGET_OBJECTS:${TARGET}>>,EXCLUDE,${regex}>
        PARENT_SCOPE)
  endif()
endfunction()

# This function is able to collect a bunch of static liberries (.a) and bundle
# them into a shared library. All libraries must be defined cmake targets, ie
# defined using the add_library() command
#
# Usage: uic_add_shared_library( <output_lib> <lib1> <lib2> ... <libN>)
function(uic_add_shared_library)
  # Parse the arguments
  list(POP_FRONT ARGV LIBNAME)
  set(LIBS ${ARGV})

  if(APPLE)
    set(LINK_OPTIONS_START -Wl,-undefined -Wl,dynamic_lookup -Wl,-all_load)
    set(LINK_OPTIONS_END "")
  else()
    set(LINK_OPTIONS_START -Wl,--whole-archive)
    set(LINK_OPTIONS_END -Wl,--no-whole-archive)
  endif()

  # Gather a list of genrator expressions which can resolve the achive file
  # belonging to the target
  foreach(L ${LIBS})
    get_target_property(TARGET_TYPE ${L} TYPE)

    list(APPEND GENERATOR_EXPRS "$<TARGET_FILE:${L}>")
  endforeach()

  # We just make an empty C file to keep Cmake happy
  set(DUMMYFILE ${CMAKE_CURRENT_BINARY_DIR}/${LIBNAME}_dummy.c)
  file(TOUCH ${DUMMYFILE})
  add_library(${LIBNAME} SHARED ${DUMMYFILE})
  add_dependencies(${LIBNAME} ${LIBS})

  target_link_options(${LIBNAME} PRIVATE ${LINK_OPTIONS_START}
                      ${GENERATOR_EXPRS} ${LINK_OPTIONS_END})
endfunction()

if(NOT TARGET sl_status_strings)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/include/sl_status_strings.h
    COMMAND bash ${DIR_OF_UIC_HELPER_CMAKE}/../../scripts/sl_status_strings.sh >
            ${CMAKE_CURRENT_BINARY_DIR}/include/sl_status_strings.h
    DEPENDS ${DIR_OF_UIC_HELPER_CMAKE}/../../include/sl_status.h
            ${DIR_OF_UIC_HELPER_CMAKE}/../../scripts/sl_status_strings.sh)
  add_custom_target(
    sl_status_strings
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/include/sl_status_strings.h)
endif()
