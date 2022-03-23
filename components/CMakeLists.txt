add_subdirectory(testframework)
add_subdirectory(uic_definitions)
add_subdirectory(uic_attribute_mapper)
add_subdirectory(uic_attribute_resolver)
add_subdirectory(uic_attribute_store)
add_subdirectory(uic_attribute_utils)
add_subdirectory(unify_attribute_poll)
add_subdirectory(uic_config)
add_subdirectory(uic_contiki)
add_subdirectory(uic_datastore)
add_subdirectory(uic_dotdot)
add_subdirectory(uic_dotdot_mqtt)
add_subdirectory(uic_log)
add_subdirectory(uic_main)
add_subdirectory(uic_main_fd)
add_subdirectory(uic_mqtt)
add_subdirectory(unify_validator)
add_subdirectory(uic_multi_invoke)
add_subdirectory(uic_smartstart_management)
add_subdirectory(uic_stdin)
add_subdirectory(uic_rust)
add_subdirectory(uic_ota)
add_subdirectory(uic_gbl_interface)

get_recursive_targets(targets ${CMAKE_CURRENT_SOURCE_DIR} True)
foreach(target ${targets})
  get_target_property(target_type ${target} TYPE)
  if(target_type MATCHES "UTILITY")
    continue()
  endif()
  if(${target} MATCHES "stub\$")
    message(STATUS "Not installing target" ${target} "to libuic as its stub")
    continue()
  endif()
  if(NOT
     (target_type MATCHES "SHARED_LIBRARY"
      OR target_type MATCHES "EXECUTABLE"
      OR target_type MATCHES "INTERFACE_LIBRARY"))
    message(
      FATAL_ERROR
        "Target \"${target}\" is not a SHARED_LIBRARY. \
         UIC Components are required to be build as shared libraries. change its type from ${target_type} to SHARED_LIBRARY.\
         if you absolutely sure you dont require a shared library, you can disable this check with function \"disable_target_check(${target})\""
    )
  endif()
  if(NOT (${target} MATCHES "^uic_" OR ${target} MATCHES "^unify_"))
    message(
      FATAL_ERROR
        "UIC libraries need to start with uic_ or unify_. Update target \"${target}\" to \"uic_${target}\" or \"unify_${target}\"!"
    )
  endif()

  install(TARGETS ${target} LIBRARY DESTINATION lib COMPONENT libuic)
endforeach()

set(description "Unified IOT Controller library")
cpack_add_component(
  libuic
  DISPLAY_NAME "libuic"
  DESCRIPTION ${description}
  INSTALL_TYPES Full)

list(LENGTH targets length)
message(STATUS "found ${length} uic target(s)")
