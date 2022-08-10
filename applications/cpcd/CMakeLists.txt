find_package(CPCD REQUIRED)

# Package as .debian
add_component_to_uic(
  uic-cpcd # Package Name
  "Universal IOT controller - CPCd"
  "${CMAKE_PROJECT_NAME}-cpcd" # Package Debian filename
  "libmbedtls12, libmbedx509-0" # Package Depends on
  "${CMAKE_BINARY_DIR}/applications/cpcd/debconf/config;\
    ${CMAKE_BINARY_DIR}/applications/cpcd/debconf/templates;\
    ${CMAKE_BINARY_DIR}/applications/cpcd/debconf/conffiles;\
    ${CMAKE_BINARY_DIR}/applications/cpcd/debconf/postinst;\
    ${CMAKE_BINARY_DIR}/applications/cpcd/debconf/prerm")


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/config.in
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/config)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/postinst.in
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/postinst)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/prerm.in
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/prerm)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/postrm.in
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/postrm)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/templates.in
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/templates)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/conffiles.in
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/conffiles)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debconf/services/uic-cpcd.service.in
               ${CMAKE_CURRENT_BINARY_DIR}/debconf/services/uic-cpcd.service)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/debconf/services/uic-cpcd.service
  DESTINATION /lib/systemd/system/
  COMPONENT uic-cpcd)

# Install binaries they come from custom target build_cpcd
install(
   TARGETS cpc
   PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}"
   PERMISSIONS
     OWNER_READ
     OWNER_WRITE
     OWNER_EXECUTE
     GROUP_EXECUTE
     GROUP_READ
     WORLD_READ
     WORLD_EXECUTE
   COMPONENT uic-cpcd)

install(
   TARGETS cpc
   LIBRARY DESTINATION "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}"
   PERMISSIONS
     OWNER_READ
     OWNER_WRITE
     OWNER_EXECUTE
     GROUP_EXECUTE
     GROUP_READ
     WORLD_READ
     WORLD_EXECUTE
   COMPONENT uic-cpcd)

install(
   TARGETS cpcd
   RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}"
   PERMISSIONS
     OWNER_READ
     OWNER_WRITE
     OWNER_EXECUTE
     GROUP_EXECUTE
     GROUP_READ
     WORLD_READ
     WORLD_EXECUTE
   COMPONENT uic-cpcd)

# Install configuration file
install(FILES "${cpcd_SOURCE_DIR}/cpcd.conf" DESTINATION "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_SYSCONFDIR}" COMPONENT uic-cpcd)

