add_executable(demo_uic_lib demo_uic_cli.cpp main.c)
target_link_libraries(demo_uic_lib PUBLIC unify)

target_compile_definitions(
  demo_uic_lib PRIVATE CMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}"
                       CMAKE_PROJECT_VERSION="${CMAKE_PROJECT_VERSION}")

# Uncomment for adding app to install package install(TARGETS demo_uic_lib
# RUNTIME DESTINATION bin) install(DIRECTORY DESTINATION
# ${CMAKE_INSTALL_PREFIX}/var/lib/demo_uic_lib)
