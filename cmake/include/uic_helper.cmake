function(get_recursive_targets result curdir omit_mocks)
  # Currently only globbing direct sub-directories (faster)!! no recursive
  # childs!
  file(
    GLOB children
    LIST_DIRECTORIES true
    ${curdir} ${curdir}/*)
  foreach(child ${children})
    if(IS_DIRECTORY "${child}" AND EXISTS "${child}/CMakeLists.txt")
      # if you get an error on the get_directory_proeperty, double check the
      # directory is included in the Cmakelists.txt tree!
      get_directory_property(targets DIRECTORY "${child}" BUILDSYSTEM_TARGETS)
      if(DEFINED omit_mocks)
        list(FILTER targets EXCLUDE REGEX ".*mock.*")
      endif()
      list(APPEND target_list ${targets})
    endif()
  endforeach()

  get_property(disabled_targets GLOBAL
               PROPERTY disabled_uic_targets_type_checker)
  list(REMOVE_ITEM target_list ${disabled_targets})
  set(${result}
      ${target_list}
      PARENT_SCOPE)
endfunction()

function(disable_target_check target)
  set_property(GLOBAL APPEND PROPERTY disabled_uic_targets_type_checker
                                      ${target})
endfunction()

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
    if(NOT (${TARGET_TYPE} MATCHES "STATIC_LIBRARY"))
      message(FATAL_ERROR " ${L} is not a static library its a ${TARGET_TYPE}")
    endif()
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
