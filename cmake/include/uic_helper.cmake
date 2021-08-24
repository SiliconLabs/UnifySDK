function(get_recursive_targets result curdir omit_mocks)
  # Currently only globbing direct sub-directories (faster)!! no recursive childs!
  file(GLOB children LIST_DIRECTORIES true ${curdir} ${curdir}/*)
  foreach(child ${children})
    if(IS_DIRECTORY "${child}" AND EXISTS "${child}/CMakeLists.txt")
      # if you get an error on the get_directory_proeperty, double check the directory
      # is included in the Cmakelists.txt tree!
      get_directory_property(targets DIRECTORY "${child}" BUILDSYSTEM_TARGETS)
      if(DEFINED omit_mocks)
        list(FILTER targets EXCLUDE REGEX ".*mock.*")
      endif()
      list(APPEND target_list ${targets})
    endif()
  endforeach()

  get_property(disabled_targets GLOBAL PROPERTY disabled_uic_targets_type_checker)  
  list(REMOVE_ITEM target_list ${disabled_targets})
  SET(${result} ${target_list} PARENT_SCOPE)
endfunction()

function(disable_target_check target)
  set_property(GLOBAL APPEND PROPERTY disabled_uic_targets_type_checker ${target})
endfunction()


function(collect_object_files TARGET TARGET_OBJS EXCLUDES)
set(TARGET_OBJS $<TARGET_OBJECTS:${TARGET}> PARENT_SCOPE)
if (NOT "${EXCLUDES}" STREQUAL "")
  list(LENGTH EXCLUDES length)
  math(EXPR list_last "${length} - 1")
  list(GET EXCLUDES ${list_last} last_element)

  foreach(ex ${EXCLUDES})
    string(APPEND regex ".*${ex}\.o.*")
    if (${length} GREATER 1 AND NOT "${last_element}" STREQUAL "${ex}")
      string(APPEND regex "|")
    endif()
  endforeach()
  
  set(TARGET_OBJS $<FILTER:$<REMOVE_DUPLICATES:$<TARGET_OBJECTS:${TARGET}>>,EXCLUDE,${regex}> PARENT_SCOPE)
endif()
endfunction()
