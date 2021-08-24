
find_package(Doxygen)

if (DOXYGEN_FOUND)
    # Template Doxyfile
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    # Final doxyfile with path substitutions
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # This variable is substituted when copying DOXYGEN_IN to DOXYGEN_OUT
    set(LIBS2_DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})

    # Copy configure file to build folder, and modify it with a reference 
    # to output into the build folder
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/main.dox.in ${CMAKE_CURRENT_BINARY_DIR}/main.dox @ONLY)

    # Run doxygen from src folder to it picks up style sheets etc.
    add_custom_target( libs2_doxygen 
         #ALL   # Uncomment this to build with any target, not just 'make all'
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        COMMAND zip -r libs2_docs.zip html
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )
else (DOXYGEN_FOUND)
  message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)
