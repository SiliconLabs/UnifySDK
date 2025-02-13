message(STATUS "Components of Unify which will have deb packages"
               ": ${CPACK_COMPONENTS_ALL}")

if(PROJECT_IS_TOP_LEVEL)
  message(STATUS "cpack: Included from ${CMAKE_SOURCE_DIR}")
  include(CPack)

  foreach(PKG_NAME IN LISTS CPACK_COMPONENTS_ALL)
    string(TOUPPER ${PKG_NAME} PKG_NAME_UPPER)
    cpack_add_component(
      PKG_NAME
      DISPLAY_NAME ${PKG_NAME}
      DESCRIPTION ${CPACK_DEBIAN_${PKG_NAME_UPPER}_DESCRIPTION}
      INSTALL_TYPES Full)
  endforeach()
endif()
