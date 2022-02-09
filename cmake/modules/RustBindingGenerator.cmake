include(CargoMeta)

rust_target_meta(
  "${PROJECT_SOURCE_DIR}/components/uic_rust/cargo-bindgen/Cargo.toml"
  CARGO_META_PACKAGE)
rust_glob_sources(${CARGO_META_PACKAGE} BINDGEN_SOURCES)
rust_output_dir(${CARGO_META_PACKAGE} BINDGEN_PLACEHOLDER)

add_custom_target(rust_bindgen)

add_custom_command(
  OUTPUT "${BINDGEN_PLACEHOLDER}"
  COMMAND ${CARGO_EXECUTABLE} "install" "-f" "--path"
          "${PROJECT_SOURCE_DIR}/components/uic_rust/cargo-bindgen"
  COMMAND "touch" "${BINDGEN_PLACEHOLDER}"
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  DEPENDS ${BINDGEN_SOURCES})
add_custom_target(rust_bindgen_install DEPENDS "${BINDGEN_PLACEHOLDER}")

function(generate_rust_bindings)
  cmake_parse_arguments(PARSED_ARGS "" "NAME;BLOCK_LIST;ALLOW_LIST"
                        "HEADERS;DEPENDS" ${ARGV})

  if(NOT PARSED_ARGS_NAME)
    message(
      FATAL_ERROR "cannot specify generate_rust_bindings without NAME argument")
  endif()

  if(NOT PARSED_ARGS_HEADERS)
    message(
      FATAL_ERROR
        "cannot specify generate_rust_bindings for ${PARSED_ARGS_NAME} without HEADERS argument"
    )
  endif()

  if(PARSED_ARGS_DEPENDS)
    rust_add_native_libs("${PARSED_ARGS_DEPENDS}")
  endif()

  set(out_file ${CMAKE_BINARY_DIR}/bindings/${PARSED_ARGS_NAME}.rs)
  set(json "{\"name\": \"\",\"headers\": \"\"}")
  string(JSON json SET ${json} "name" "\"${PARSED_ARGS_NAME}\"")
  string(JSON json SET ${json} "headers"
         ["$<JOIN:${PARSED_ARGS_HEADERS},\",\">"])
  string(JSON json SET ${json} "out_file" "\"${out_file}\"")
  string(
    JSON
    json
    SET
    ${json}
    "include_dirs"
    ["$<JOIN:$<TARGET_PROPERTY:cargo_build,INTERFACE_INCLUDE_DIRECTORIES>,\",\">"]
  )

  if(PARSED_ARGS_ALLOW_LIST)
    string(JSON json SET ${json} "allow_list" "\"${PARSED_ARGS_ALLOW_LIST}\"")
  endif()
  if(PARSED_ARGS_BLOCK_LIST)
    string(JSON json SET ${json} "block_list" "\"${PARSED_ARGS_BLOCK_LIST}\"")
  endif()

  set(json_conf "[]")
  string(JSON json_conf SET ${json_conf} 999 "${json}")

  set(config_file ${CMAKE_BINARY_DIR}/bindings/${PARSED_ARGS_NAME}_config.json)
  file(
    GENERATE
    OUTPUT ${config_file}
    CONTENT ${json_conf})

  add_custom_command(
    OUTPUT ${out_file}
    COMMAND ${CARGO_EXECUTABLE} "bindgen" "--config" "${config_file}"
    DEPENDS rust_bindgen_install ${config_file})

  add_custom_target(rust_bindgen_${PARSED_ARGS_NAME}
                    DEPENDS rust_bindgen_install ${out_file})
  add_dependencies(rust_bindgen rust_bindgen_${PARSED_ARGS_NAME})
endfunction()
