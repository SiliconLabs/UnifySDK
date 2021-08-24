# * Find PlantUML jar file
#
# PlantUML_JARFILE - Path to PlantUML JAR file
# PlantUML_FOUND - True if PlantUML found.

if(NOT DEFINED PlantUML_FOUND)
  if ("$ENV{PLANTUML_JAR_PATH}")
    # Find folder of the the TEMPLATE_JSON file
    get_filename_component(PLANTUML_DIR
      ${PLANTUML_JAR_PATH} DIRECTORY)
  elseif (EXISTS /usr/share/plantuml/)
    set(PLANTUML_DIR /usr/share/plantuml/)
  endif()
  find_file(PlantUML_JARFILE
    NAMES plantuml.jar
    HINTS "" ${PLANTUML_DIR}
  )
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(PlantUML DEFAULT_MSG PlantUML_JARFILE)
endif()

