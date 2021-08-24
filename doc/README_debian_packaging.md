# README to Make Debian Package for UIC Component (for Example ZPC)

1. Implement add_component_to_uic() function in cmake/include/package.cmake for your component
with the following arguments:
    # Function arguments
    # 1. PKG_NAME      component package Name
    # 2. PKG_FILE_NAME Debian package filename for the component
    # 3. PKG_DEPNDS    Component debian package depenedencies.
    # 4. PKG_EXTRA     Component debian package Control extra info
    # See https://cmake.org/cmake/help/v3.6/module/CPackDeb.html for more information
on these fields.

See the implementation of the ZPC component.

2. In the component-specific CMakeFile.txt (for application/zpc/CMakeFile.txt)
implement a call to cpack_add_component() for filling up the CPack component
information.

3. Pass "COMPONENT \<component name\> as an argument to all install() calls for the
component (see the application/zpc/CMakeFile.txt for example).

4. Add your component name to the following line, as shown in
cmake/include/package.cmake.
  ~~~ bash
  set(CPACK_COMPONENTS_ALL uic-zpc uic-dev_cli libuic uic-all <your component>)
  ~~~

5. Add your component name to uic-all component (append name to this list as
shown in cmake/include/package.cmake).
  ~~~ bash
  add_component_to_uic(
    uic-all
    "${CMAKE_PROJECT_NAME}-all"
    "uic-zpc,uic-dev_cli,libuic, <your component>"
    ""
    )
  ~~~
