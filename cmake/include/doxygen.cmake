find_package(Doxygen)

if(DOXYGEN_FOUND)
  find_package(PlantUML)
  # ############################################################################
  # Shared doxygen options
  # ############################################################################
  # Excluding a few large items to reduce the doxygen build time
  set(DOXYGEN_FILE_PATTERNS *.h *.c *.dox *.doxygen *.hpp *.md)
  set(DOXYGEN_EXCLUDE_PATTERNS
      */test/* */libs/* ZW_classcmd.h */uic_dotdot/**.md */gen/*
      */uic_attribute_mapper/*/*.md)
  set(DOXYGEN_HTML_EXTRA_STYLESHEET
      ${CMAKE_CURRENT_SOURCE_DIR}/doc/doxygen/assets/customdoxygen.css)
  set(DOXYGEN_HTML_EXTRA_FILES
      ${CMAKE_CURRENT_SOURCE_DIR}/doc/doxygen/assets/silicon-labs-logo.png)
  set(DOXYGEN_EXTRACT_ALL YES)
  set(DOXYGEN_GENERATE_TREEVIEW YES)
  # set(DOXYGEN_GENERATE_XML)

  set(DOXYGEN_GENERATE_LATEX YES)
  set(DOXYGEN_LATEX_OUTPUT latex)
  set(DOXYGEN_BATCHMODE YES)
  set(DOXYGEN_USE_PDFLATEX YES)
  set(DOXYGEN_LATEX_CMD_NAME "\"pdflatex -interaction=nonstopmode\"")

  set(DOXYGEN_HIDE_UNDOC_RELATIONS NO)
  set(DOXYGEN_BUILTIN_STL_SUPPORT YES)  
  set(DOXYGEN_EXTRACT_PRIVATE YES)
  set(DOXYGEN_EXTRACT_PACKAGE YES)
  set(DOXYGEN_EXTRACT_STATIC YES)
  set(DOXYGEN_EXTRACT_LOCALMETHODS YES)
  set(DOXYGEN_UML_LOOK YES)
  set(DOXYGEN_UML_LIMIT_NUM_FIELDS 50)
  set(DOXYGEN_TEMPLATE_RELATIONS YES)
  set(DOXYGEN_HTML_TIMESTAMP YES)
  set(DOXYGEN_WARN_NO_PARAMDOC YES)

  # Set HAVE_DOT to NO as default to turn off generating graphs (reduces build
  # time) If graphs are desired set DOXYGEN_HAVE_DOT to YES with
  # -DDOXYGEN_HAVE_DOT=YES at cmake config time
  if(NOT DEFINED DOXYGEN_HAVE_DOT)
    message(
      STATUS
        "Doxygen configured without dot, if you desire graphs run cmake with -DDOXYGEN_HAVE_DOT=YES"
    )
    set(DOXYGEN_HAVE_DOT NO)
  endif()
  set(DOXYGEN_DOT_GRAPH_MAX_NODES 100)
  set(DOXYGEN_MAX_DOT_GRAPH_DEPTH 0)
  # If HAVE_DOT = YES we will generate all graphs - probably not what we want
  set(DOXYGEN_DOT_TRANSPARENT YES)
  set(DOXYGEN_DOT_IMAGE_FORMAT "svg")
  set(DOXYGEN_CLASS_DIAGRAMS YES)
  set(DOXYGEN_COLLABORATION_GRAPH YES)
  set(DOXYGEN_CLASS_GRAPH YES)
  set(DOXYGEN_CALL_GRAPH YES)
  set(DOXYGEN_CALLER_GRAPH YES)
  set(DOXYGEN_GROUP_GRAPHS YES)
  set(DOXYGEN_INCLUDE_GRAPH YES)
  set(DOXYGEN_INCLUDED_BY_GRAPH YES)
  set(DOXYGEN_GRAPHICAL_HIERARCHY YES)
  set(DOXYGEN_DIRECTORY_GRAPH YES)
  set(DOXYGEN_VERBATIM_VARS DOXYGEN_ALIASES)
  set(DOXYGEN_LATEX)
  if(PlantUML_FOUND)
    set(DOXYGEN_PLANTUML_JAR_PATH ${PlantUML_JARFILE})
  else()
    message(
      WARNING "PlantUML NOT found, Doxygen documentation will be incomplete")
  endif()
  # Doxygen src for libuic
  set(LIBUIC_DOXYGEN_SRC
      ${CMAKE_SOURCE_DIR}/components
      ${CMAKE_CURRENT_BINARY_DIR}/components
      ${CMAKE_SOURCE_DIR}/include
      ${CMAKE_SOURCE_DIR}/doc/standards
      ${CMAKE_SOURCE_DIR}/doc/readme_developer.md
      ${CMAKE_SOURCE_DIR}/doc/overview.md
      ${CMAKE_SOURCE_DIR}/docker/readme_developer.md
      ${CMAKE_SOURCE_DIR}/doc/README_debian_packaging.md
      ${CMAKE_SOURCE_DIR}/doc/readme_uic_application_developer.md)

  # ############################################################################
  # Create doxygen build target, that includes all other doxygen targets
  # ############################################################################
  add_custom_target(doxygen)
  add_custom_target(doxygen_zip)
  add_custom_target(doxygen_pdf)

  function(add_doxygen_target)
    # DOX_TARGET DOX_PROJECT_NAME DOX_IMAGE_PATH DOX_SOURCES
    cmake_parse_arguments(
      ADD_DOX
      "PDF" # Boolean Flags
      "TARGET;PROJECT_NAME" # Mono value arguments
      "SRC_PATHS;IMAGE_PATHS;PLANTUML_PATHS" # Multi-value arguments
      ${ARGN})

    set(DOXYGEN_OUTPUT_DIRECTORY ${ADD_DOX_TARGET})
    set(DOXYGEN_WARN_LOGFILE
        ${CMAKE_CURRENT_BINARY_DIR}/${DOXYGEN_OUTPUT_DIRECTORY}/${ADD_DOX_TARGET}_log.txt
    )
    set(DOXYGEN_PROJECT_NAME ${ADD_DOX_PROJECT_NAME})
    if(ADD_DOX_IMAGE_PATHS)
      message(STATUS Setting doxygen images path to ${ADD_DOX_IMAGE_PATHS})
      set(DOXYGEN_IMAGE_PATH ${ADD_DOX_IMAGE_PATHS})
    endif()
    if(ADD_DOX_PLANTUML_PATHS)
      if(PlantUML_FOUND)
        set(DOXYGEN_PLANTUML_INCLUDE_PATH ${ADD_DOX_IMAGE_PATHS})
      endif()
    endif()
    doxygen_add_docs(${ADD_DOX_TARGET} ${ADD_DOX_SRC_PATHS})
    # Print doxygen warnings after doxygen is built
    add_custom_command(TARGET ${ADD_DOX_TARGET} COMMAND cat
                                                        ${DOXYGEN_WARN_LOGFILE})
    add_dependencies(doxygen ${ADD_DOX_TARGET})
    add_custom_target(
      ${ADD_DOX_TARGET}_zip
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${ADD_DOX_TARGET}
      DEPENDS ${ADD_DOX_TARGET}
      COMMAND zip -r -q ${ADD_DOX_TARGET}_${CMAKE_PROJECT_VERSION}.docs.zip
              html)
    
    if (${ADD_DOX_PDF}) 
      add_custom_target(
        ${ADD_DOX_TARGET}_pdf
        DEPENDS 
          ${ADD_DOX_TARGET}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${ADD_DOX_TARGET}/latex
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/build/pdf_latex/compile_latex.sh ${ADD_DOX_TARGET}_${CMAKE_PROJECT_VERSION}
      )

      add_dependencies(doxygen_pdf ${ADD_DOX_TARGET}_pdf)
    endif()        

    add_dependencies(doxygen_zip ${ADD_DOX_TARGET}_zip)
  endfunction()
  # ############################################################################
  # Configure Doxygen for libuic
  # ############################################################################
  add_doxygen_target(
    TARGET
    doxygen_uic
    PROJECT_NAME
    "Unify SDK Lib"
    IMAGE_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/doc/assets/img/
    SRC_PATHS
    ${CMAKE_SOURCE_DIR}/doc/doxygen
    ${CMAKE_BINARY_DIR}/components/dotdot_mqtt/src
    ${LIBUIC_DOXYGEN_SRC})
  # ############################################################################
  # Configure Doxygen for ZPC
  # ############################################################################
  # The aliases syntax are pretty ugly. Want to use DOXYGEN_VERBATIM_VARS
  # feature but it's only available with CMake v3.11+
  set(DOXYGEN_ALIASES
      "serial_rx{1}=> *Serial API (ZW &rarr\; host):*&nbsp\;&nbsp\; `\\1`<br/>"
      "serial_tx{1}=> *Serial API (host &rarr\; ZW):*&nbsp\;&nbsp\; `\\1`<br/>"
      "zgw_name=\"@xrefitem zgw_namemap \\\"Legacy Z/IP Gateway Serial API Name\\\" \\\"Legacy Z/IP Gateway Serial API Names\\\"\""
  )

  add_doxygen_target(
    TARGET
    doxygen_zpc
    PROJECT_NAME
    "Z-Wave Protocol Controller"
    IMAGE_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/doc/assets/img/
    ${CMAKE_CURRENT_SOURCE_DIR}/applications/zpc/doc/assets/img/
    PLANTUML_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/applications/zpc/doc/assets/plantuml/
    SRC_PATHS
    ${CMAKE_BINARY_DIR}/components/dotdot_mqtt/src
    ${CMAKE_CURRENT_SOURCE_DIR}/applications/zpc
    ${CMAKE_BINARY_DIR}/components/dotdot_mqtt/src
    ${CMAKE_BINARY_DIR}/applications/zpc/components/zwave_command_classes/src-gen
    ${LIBUIC_DOXYGEN_SRC})
  # ############################################################################
  # Configure Doxygen for dev_cli
  # ############################################################################
  add_doxygen_target(
    TARGET
    doxygen_dev_cli
    PROJECT_NAME
    "Developer Command Line Interface"
    IMAGE_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/doc/assets/img/
    SRC_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/applications/dev_ui/dev_cli
    ${LIBUIC_DOXYGEN_SRC})
  # ############################################################################
  # Configure Doxygen for zigpc
  # ############################################################################
  add_doxygen_target(
    TARGET
    doxygen_zigpc
    PROJECT_NAME
    "Zigbee Protocol Controller"
    IMAGE_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/doc/assets/img/
    SRC_PATHS
    ${CMAKE_SOURCE_DIR}/applications/zigpc
    ${CMAKE_CURRENT_BINARY_DIR}/applications/zigpc/components/zcl_command_parser/include
    ${CMAKE_CURRENT_BINARY_DIR}/applications/zigpc/components/zcl_util/include
    ${LIBUIC_DOXYGEN_SRC})
  # ############################################################################
  # Configure Doxygen for User Guides
  # ############################################################################
  set(DOXYGEN_USE_MDFILE_AS_MAINPAGE README.md)
  file(GLOB USERGUIDE_MD_FILES1 applications/*/*.md LIST_DIRECTORIES true)
  list(FILTER USERGUIDE_MD_FILES1 EXCLUDE REGEX "zigpc.md$")
  file(GLOB USERGUIDE_MD_FILES2 applications/dev_ui/dev_gui/*.md
       LIST_DIRECTORIES true)

  add_doxygen_target(
    TARGET
    userguide
    PROJECT_NAME
    "Unify SDK User Guide"
    IMAGE_PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/doc/assets/img/
    ${CMAKE_CURRENT_SOURCE_DIR}/applications/dev_ui/dev_gui/doc/assets/img/
    ${CMAKE_CURRENT_SOURCE_DIR}/applications/zpc/doc/assets/img/
    ${CMAKE_CURRENT_SOURCE_DIR}/applications/zigpc
    SRC_PATHS
    README.md
    doc/readme_developer.md
    doc/overview.md
    doc/readme_uic_application_developer.md
    docker/readme_developer.md
    ${USERGUIDE_MD_FILES2}
    ${USERGUIDE_MD_FILES1}
    doc/standards/known-abbreviations.md
    PDF 
    true)
  unset(DOXYGEN_USE_MDFILE_AS_MAINPAGE)

else()
  message(
    STATUS "Doxygen need to be installed to generate the doxygen documentation")
endif()
