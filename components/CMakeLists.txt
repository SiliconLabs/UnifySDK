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
add_subdirectory(uic_dotdot_attribute_store)
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
add_subdirectory(unify_node_state_monitor)

get_recursive_targets(targets ${CMAKE_CURRENT_SOURCE_DIR} True)
list(LENGTH targets length)
message(STATUS "found ${length} uic target(s)")
set(UIC_LIBS "" )
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
         Unify Components are required to be build as shared libraries. change its type from ${target_type} to SHARED_LIBRARY.\
         if you absolutely sure you dont require a shared library, you can disable this check with function \"disable_target_check(${target})\""
    )
  endif()
  if(NOT (${target} MATCHES "^uic_" OR ${target} MATCHES "^unify_"))
    message(
      FATAL_ERROR
        "Unify libraries need to start with uic_ or unify_. Update target \"${target}\" to \"uic_${target}\" or \"unify_${target}\"!"
    )
  endif()

  install(TARGETS ${target} LIBRARY DESTINATION lib COMPONENT libuic)
  list(APPEND UIC_LIBS "${target}")
endforeach()

string (REPLACE ";" " " UIC_LIBS "${UIC_LIBS}")
configure_file(assets/libuic.pc.in libuic.pc @ONLY)
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/libuic.pc
  DESTINATION share/pkgconfig
  COMPONENT libuic-dev
)

file(GLOB_RECURSE PUBLIC_HEADERS
../include/*.h
../include/*.hpp
uic_*/include/*.h
uic_*/include/*.hpp
unify_*/include/*.h
unify_*/include/*.hpp
uic_contiki/core/*.h
uic_contiki/platform/posix/*.h
)

file(GLOB_RECURSE PUBLIC_CONTIKI_SYS
  uic_contiki/core/sys/*.h
  uic_contiki/core/sys/*.hpp
)

file(GLOB_RECURSE PUBLIC_CONTIKI_LIB
  uic_contiki/core/lib/*.h
  uic_contiki/core/lib/*.hpp
)

add_custom_command(
  COMMAND mkdir -p include/uic/lib
  COMMAND mkdir -p include/uic/sys
  COMMAND cp -r ${PUBLIC_HEADERS} ${CMAKE_CURRENT_BINARY_DIR}/include/uic
  COMMAND cp -r ${PUBLIC_CONTIKI_SYS} ${CMAKE_CURRENT_BINARY_DIR}/include/uic/sys
  COMMAND cp -r ${PUBLIC_CONTIKI_LIB} ${CMAKE_CURRENT_BINARY_DIR}/include/uic/lib
  OUTPUT include/uic/PUBLIC_HEADERS include/uic/sys/PUBLIC_CONTIKI_SYS include/uic/lib/PUBLIC_CONTIKI_LIB
)

add_custom_target(libuic-dev
  DEPENDS include/uic/PUBLIC_HEADERS
)

add_dependencies(libuic-dev ${targets})

install (
FILES ${PUBLIC_HEADERS}
DESTINATION include/uic
COMPONENT libuic-dev
)


add_component_to_uic(
  libuic-dev "Unified IOT Controller library - Development"
  "libuic-dev" "libuic" "")

add_component_to_uic(
  libuic "Unified IOT Controller library"
  "libuic" "libboost-program-options1.67.0,\
      libedit2, libsqlite3-0, libmosquitto1,\
      libyaml-cpp0.6, libboost-log1.67.0" "")

