#
# Documentation target_interface_libraries(<TARGET_NAME> lib1 [lib2] ... [libN])
#
# Add include path to target from each of the libs
function(target_interface_libraries TARGET)
  set(IF_LIBS ${ARGV})
  # Pop the first argument of the list
  list(REMOVE_AT IF_LIBS 0)
  foreach(IF_LIB ${IF_LIBS})
    target_include_directories(
      ${TARGET}
      PUBLIC "$<TARGET_PROPERTY:${IF_LIB},INTERFACE_INCLUDE_DIRECTORIES>")
    # Set TARGET to depend on library, required in cases, where the IF_LIB
    # generate header files that the TARGET depends on
    add_dependencies(${TARGET} ${IF_LIB})
  endforeach()
endfunction()
