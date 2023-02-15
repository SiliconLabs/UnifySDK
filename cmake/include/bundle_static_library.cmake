# MIT License
#
# Copyright (c) 2019 Cristian Adam Copyright (c) 2021 William O'Neill
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

function(bundle_static_library tgt_name bundled_tgt_name)
  list(APPEND static_libs ${tgt_name})

  function(_recursively_collect_dependencies input_target)
    set(_input_link_libraries LINK_LIBRARIES)
    get_target_property(_input_type ${input_target} TYPE)
    if(${_input_type} STREQUAL "INTERFACE_LIBRARY")
      set(_input_link_libraries INTERFACE_LINK_LIBRARIES)
    endif()
    get_target_property(public_dependencies ${input_target}
                        ${_input_link_libraries})
    foreach(dep IN LISTS public_dependencies)
      # Added to support out-of-directory target link library settings. Not sure
      # if this should need extra processing of the source directory, but it is
      # working for us when we make sure bundle_static_library is called last
      string(REGEX REPLACE "::@.*" "" dependency ${dep})
      if(TARGET ${dependency})
        get_target_property(alias ${dependency} ALIASED_TARGET)
        if(TARGET ${alias})
          set(dependency ${alias})
        endif()
        get_target_property(_type ${dependency} TYPE)
        if(${_type} STREQUAL "STATIC_LIBRARY")
          list(APPEND static_libs ${dependency})
        endif()
        if(${_type} STREQUAL "OBJECT_LIBRARY")
          list(APPEND object_libs ${dependency})
        endif()

        get_property(library_already_added GLOBAL
                     PROPERTY _${tgt_name}_static_bundle_${dependency})
        if(NOT library_already_added)
          set_property(GLOBAL PROPERTY _${tgt_name}_static_bundle_${dependency}
                                       ON)
          _recursively_collect_dependencies(${dependency})
        endif()
      endif()
    endforeach()
    set(static_libs
        ${static_libs}
        PARENT_SCOPE)
    set(object_libs
        ${object_libs}
        PARENT_SCOPE)
  endfunction()

  _recursively_collect_dependencies(${tgt_name})

  list(REMOVE_DUPLICATES static_libs)
  list(REMOVE_DUPLICATES object_libs)

  set(bundled_tgt_full_name
      ${CMAKE_BINARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}${bundled_tgt_name}${CMAKE_STATIC_LIBRARY_SUFFIX}
  )

  if(CMAKE_CXX_COMPILER_ID MATCHES "(Clang|GNU)")
    file(WRITE ${CMAKE_BINARY_DIR}/${bundled_tgt_name}.ar.in
         "CREATE ${bundled_tgt_full_name}\n")

    foreach(tgt IN LISTS static_libs)
      file(APPEND ${CMAKE_BINARY_DIR}/${bundled_tgt_name}.ar.in
           "ADDLIB $<TARGET_FILE:${tgt}>\n")
    endforeach()

    foreach(tgt IN LISTS object_libs)
      file(APPEND ${CMAKE_BINARY_DIR}/${bundled_tgt_name}.ar.in
           "ADDMOD $<TARGET_OBJECTS:${tgt}>\n")
    endforeach()

    file(APPEND ${CMAKE_BINARY_DIR}/${bundled_tgt_name}.ar.in "SAVE\n")
    file(APPEND ${CMAKE_BINARY_DIR}/${bundled_tgt_name}.ar.in "END\n")

    file(
      GENERATE
      OUTPUT ${CMAKE_BINARY_DIR}/${bundled_tgt_name}.ar
      INPUT ${CMAKE_BINARY_DIR}/${bundled_tgt_name}.ar.in)

    set(ar_tool ${CMAKE_AR})
    set(ranlib_tool ${CMAKE_RANLIB})
    if(CMAKE_INTERPROCEDURAL_OPTIMIZATION)
      set(ar_tool ${CMAKE_CXX_COMPILER_AR})
      set(ranlib_tool ${CMAKE_CXX_COMPILER_RANLIB})
    endif()

    # TODO: confirm ranlib is required (adding this b/c of objlib additions)
    add_custom_command(
      COMMAND ${ar_tool} -M < ${CMAKE_BINARY_DIR}/${bundled_tgt_name}.ar &&
              ${ranlib_tool} ${bundled_tgt_full_name}
      OUTPUT ${bundled_tgt_full_name}
      COMMENT "Bundling ${bundled_tgt_name}"
      VERBATIM)
  elseif(MSVC)
    # IMPORTANT: objlib support NOT added for MSVC presumably something needs
    # done with object_libs list
    find_program(lib_tool lib)

    foreach(tgt IN LISTS static_libs)
      list(APPEND static_libs_full_names $<TARGET_FILE:${tgt}>)
    endforeach()

    add_custom_command(
      COMMAND ${lib_tool} /NOLOGO /OUT:${bundled_tgt_full_name}
              ${static_libs_full_names}
      OUTPUT ${bundled_tgt_full_name}
      COMMENT "Bundling ${bundled_tgt_name}"
      VERBATIM)
  else()
    message(FATAL_ERROR "Unknown bundle scenario!")
  endif()

  add_custom_target(bundling_target ALL DEPENDS ${bundled_tgt_full_name})
  add_dependencies(bundling_target ${tgt_name})

  message(STATUS "Creating ${bundled_tgt_name} lib")
  add_library(${bundled_tgt_name} STATIC IMPORTED)
  set_target_properties(
    ${bundled_tgt_name}
    PROPERTIES IMPORTED_LOCATION ${bundled_tgt_full_name}
               INTERFACE_INCLUDE_DIRECTORIES
               $<TARGET_PROPERTY:${tgt_name},INTERFACE_INCLUDE_DIRECTORIES>)
  add_dependencies(${bundled_tgt_name} bundling_target)

endfunction()
