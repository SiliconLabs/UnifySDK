# CMake coding standard 

This page describes the coding standard for all cmake files, incuding both CMakeLists.txt files and *.cmake files.

* All files must have a line wrap of 80 characters.
* Indention is 2 spaces.
* All cmake functions must be written in lowercase, but keywords of keyword arguments must be written in uppercase. 
* All variables must be written in uppercase.
* All functions must use keyword arguments if available, with an indention of 2 spaces.
* Keyword arguments must be written on a separate line.

Example:
```
 add_custom_target(
    clean-gcov
    COMMAND find ${PROJECT_BINARY_DIR} -name \\*.gcda -exec rm {} \\\\; \\;
    COMMAND find ${PROJECT_BINARY_DIR} -name \\*.gcno -exec rm {} \\\\;
    COMMENT "Cleaning GCOV statistics")
```
* If a function call is less than 80 characters the entire call must be on a single line.

Example:
```
add_program(foobar foo.c bar.c)
```

All cmake functions must be documented, use of markdown style lists should be used
where applicable.
Example:
```
# Usage:
#   foo(<TARGET_NAME> SOURCES <sources> HEADERS <headers>)
# 
# * SOURCES List of source files
# * HEADERS List of header files.
#
function(FOO)

...
...

endfunction()
```




