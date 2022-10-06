# ZAP Generation By default no ZAP generation be done unless CMAKE option
# ZAP_GENERATE is set to ON ZAP generated files are placed in ZAP_OUTPUT_FOLDER
option(ZAP_GENERATE "Generate ZAP files" OFF)
if (NOT ZAP_CMAKE)
  set(ZAP_CMAKE True)
else()
  return()
endif()

if(ZAP_GENERATE MATCHES ON)
  add_custom_target(zap)
  find_program(ZAP_EXECUTABLE "zap" REQUIRED)
  set(ZAP_GENERATE_CMD_STR
      "${ZAP_EXECUTABLE} generate --noUi --noServer --no-sandbox --tempState")
  if(DEFINED ENV{HEADLESS_HOST})
    find_program(XVFB_RUN_EXECUTABLE "xvfb-run" REQUIRED)
    message(
      STATUS
        "Found ENV(HEADLESS_HOST), using ${XVFB_RUN_EXECUTABLE} to run ZAP headless"
    )
    string(PREPEND ZAP_GENERATE_CMD_STR "${XVFB_RUN_EXECUTABLE} -a ")
  else()
    message(
      STATUS "Missing ENV(HEADLESS_HOST), running zap will require a display")
  endif()

  set(DIR_OF_ZAP_CMAKE ${CMAKE_CURRENT_LIST_DIR} CACHE INTERNAL "DIR_OF_ZAP_CMAKE")
  set(ZAP_GENERATE_CMD_STR
      ${ZAP_GENERATE_CMD_STR}
      CACHE INTERNAL zap_generate_cmd_str)
  message(STATUS "Using zap generate command: ${ZAP_GENERATE_CMD_STR}")
endif()

# Documentation RUN_ZAP(TEMPLATE_JSON)
#
# This macro parse the TEMPLATE_JSON and create a custom command, that runs ZAP
# It adds dependencies to all input files from the JSON file, and creates output
# files for each entry in the "templates" field in the json file.
#
# NB: ZAP Generation only takes place if ZAP_GENERATE is set to ON After calling
# run_zap macro ZAP_CURRENT_OUTPUT_DIR variable is set to the dir where the ZAP
# generated files are located.
macro(RUN_ZAP TEMPLATE_JSON)
  set(ZAP_CURRENT_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/zap-generated")
  if(ZAP_GENERATE MATCHES ON)
    # Find folder of the the TEMPLATE_JSON file
    get_filename_component(
      template_folder ${CMAKE_CURRENT_SOURCE_DIR}/${TEMPLATE_JSON} DIRECTORY)

    # Read the ${CMAKE_CURRENT_SOURCE_DIR}/${TEMPLATE_JSON} and store it in
    # ZAP_TEMPLATE_JSON
    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/${TEMPLATE_JSON} ZAP_TEMPLATE_JSON)

    # Parse the TEMPLATE_JSON file to and get "path" and "output" for each of
    # the entries in "templates", this is used to set OUTPUT and DEPENDS so
    # cmake will detect changes to these files and run ZAP accordingly
    string(JSON zap_template_count LENGTH ${ZAP_TEMPLATE_JSON} templates)
    # Subtract 1 from the number of templates, because the cmake foreach
    # includes includes the <stop> in the loop. e.g. foreach(index 1) runs with
    # index 0 and 1
    math(EXPR zap_template_count "${zap_template_count}-1")
    foreach(index RANGE ${zap_template_count})
      # Read templates[<index>]["path"] from json
      string(
        JSON
        z_path
        GET
        ${ZAP_TEMPLATE_JSON}
        templates
        ${index}
        path)
      # Read templates[<index>]["output"] from json
      string(
        JSON
        z_output
        GET
        ${ZAP_TEMPLATE_JSON}
        templates
        ${index}
        output)

      # Append to input list from "path"
      list(APPEND zap_input ${template_folder}/${z_path})
      # Append to output list from "output"
      list(APPEND zap_output ${ZAP_CURRENT_OUTPUT_DIR}/${z_output})
      # Ensure output directory exists, as ZAP fails if the folder doesn't exist
      get_filename_component(out_folder ${ZAP_CURRENT_OUTPUT_DIR}/${z_output}
                             DIRECTORY)
      file(MAKE_DIRECTORY ${out_folder})
    endforeach()

    # Parse override from TEMPLATE_JSON and add to zap_input list (dependency)
    string(JSON zap_override GET ${ZAP_TEMPLATE_JSON} override)
    if(NOT zap_override STREQUAL "")
      list(APPEND zap_input ${template_folder}/${zap_override})
    endif()

    # Parse helpers from TEMPLATE_JSON and add to zap_input list (dependency)
    string(JSON zap_helpers_type TYPE ${ZAP_TEMPLATE_JSON} helpers)
    if(zap_helpers_type STREQUAL ARRAY)
      string(JSON zap_helpers_count LENGTH ${ZAP_TEMPLATE_JSON} helpers)
      math(EXPR zap_helpers_count "${zap_helpers_count}-1")
      foreach(index RANGE ${zap_helpers_count})
        string(JSON zap_helper GET ${ZAP_TEMPLATE_JSON} helpers ${index})
        list(APPEND zap_input ${template_folder}/${zap_helper})
      endforeach()
    endif()

    separate_arguments(ZAP_GENERATE_CMD_ARGS NATIVE_COMMAND
                       ${ZAP_GENERATE_CMD_STR})
    add_custom_command(
      OUTPUT ${zap_output}
      DEPENDS ${zap_input}
      COMMAND
        ${ZAP_GENERATE_CMD_ARGS} -z
        ${DIR_OF_ZAP_CMAKE}/../../components/uic_dotdot/dotdot-xml/library.xml -g
        ${CMAKE_CURRENT_SOURCE_DIR}/${TEMPLATE_JSON} -o
        ${ZAP_CURRENT_OUTPUT_DIR})
    # Create a custom target for running ZAP on current tamplate based on the
    # path to the output folder.
    string(REGEX REPLACE "${CMAKE_SOURCE_DIR}/" "" ZAP_CURRENT_DIR_RELATIVE
                         ${ZAP_CURRENT_OUTPUT_DIR})
    string(REGEX REPLACE "[^A-Za-z0-9\_\-]" "_" zap_target_name
                         ${ZAP_CURRENT_DIR_RELATIVE})
    message(STATUS "Adding ZAP target: ${zap_target_name}")
    # Add custom target for running ZAP on the current template.
    add_custom_target(${zap_target_name} DEPENDS ${zap_output})
    # Add dependency to "zap" target, so all zap generations can be executed by
    # this target.
    add_dependencies(zap ${zap_target_name})
  endif()
endmacro()
