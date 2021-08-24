
###############################################################################
# CROSS_SYSTEM_DIR(IN_SEARCH_PATH IN_MATCH_PATTERN OUT_CROSS_SYSTEM_DIR)
#
# Function for searching and returning the correct folder during cross compilation
# Folders are returned according to first match:
# If IN_MATCH_PATTERN is empty string, i.e. "", then the first matched cross folder is returned.
# 1) ${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}: If two level folder with same name exists.
# 2) ${CMAKE_SYSTEM_NAME}/generic: If ${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR} does not exists but folder ${CMAKE_SYSTEM_NAME}/${IN_MATCH_PATTERN}/generic exists.
# 3) ${CMAKE_SYSTEM_NAME}/: If folder with same name exists.
# 4) ${CMAKE_SYSTEM_PROCESSOR}: If folder with same name exists.
# 5) generic: If ${CMAKE_SYSTEM_PROCESSOR} does not exists but folder 'generic' exists.
# 6) ${IN_MATCH_PATTERN}: The folder itself is returned
# 7) '': If no matches are found
# For all tests above, it applies that if ${IN_MATCH_PATTERN} is set then the folder is only returned if a file/folder matching the pattern is found.
# e.g. if ${IN_MATCH_PATTERN} is set to 'include' and ${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR} does not contain an 'include' file/folder then the tests continues to 2)
# @param[in]  IN_SEARCH_PATH Name of path/folder that should be seached
# @param[in]  IN_MATCH_PATTERN  Name of subfolder/file in search path that should be found, set to "" / or empty variable if no match folder is needed
# @param[out] OUT_CROSS_SYSTEM_DIR Name of path/folder which matched above rule
###############################################################################
IF (NOT COMMAND CROSS_SYSTEM_DIR)
  FUNCTION(CROSS_SYSTEM_DIR IN_SEARCH_PATH IN_MATCH_PATTERN OUT_CROSS_SYSTEM_DIR)
    IF(NOT (IS_ABSOLUTE ${IN_SEARCH_PATH}))
      SET(IN_SEARCH_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${IN_SEARCH_PATH}")
    ENDIF(NOT (IS_ABSOLUTE ${IN_SEARCH_PATH}))

    IF((CMAKE_SYSTEM_NAME) AND (CMAKE_SYSTEM_PROCESSOR) AND (EXISTS ${IN_SEARCH_PATH}/${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}/${IN_MATCH_PATTERN}))
      SET(${OUT_CROSS_SYSTEM_DIR} "${IN_SEARCH_PATH}/${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}" PARENT_SCOPE)
    ELSEIF((CMAKE_SYSTEM_NAME) AND (EXISTS ${IN_SEARCH_PATH}/${CMAKE_SYSTEM_NAME}/generic/${IN_MATCH_PATTERN}))
      SET(${OUT_CROSS_SYSTEM_DIR} "${IN_SEARCH_PATH}/${CMAKE_SYSTEM_NAME}/generic" PARENT_SCOPE)
    ELSEIF((CMAKE_SYSTEM_NAME) AND (NOT (${CMAKE_SYSTEM_NAME} STREQUAL "Generic")) AND (EXISTS ${IN_SEARCH_PATH}/${CMAKE_SYSTEM_NAME}/${IN_MATCH_PATTERN}))
      SET(${OUT_CROSS_SYSTEM_DIR} "${IN_SEARCH_PATH}/${CMAKE_SYSTEM_NAME}" PARENT_SCOPE)
    ELSEIF((CMAKE_SYSTEM_PROCESSOR) AND (EXISTS ${IN_SEARCH_PATH}/${CMAKE_SYSTEM_PROCESSOR}/${IN_MATCH_PATTERN}))
      SET(${OUT_CROSS_SYSTEM_DIR} "${IN_SEARCH_PATH}/${CMAKE_SYSTEM_PROCESSOR}" PARENT_SCOPE)
    ELSEIF(EXISTS ${IN_SEARCH_PATH}/generic/${IN_MATCH_PATTERN})
      SET(${OUT_CROSS_SYSTEM_DIR} "${IN_SEARCH_PATH}/generic" PARENT_SCOPE)
    ELSEIF(EXISTS ${IN_SEARCH_PATH}/${IN_MATCH_PATTERN})
      SET(${OUT_CROSS_SYSTEM_DIR} "${IN_SEARCH_PATH}" PARENT_SCOPE)
    ELSE()
      IF(${IN_MATCH_PATTERN} STREQUAL "")
        MESSAGE(WARNING "CROSS_SYSTEM_DIR failed to find cross platform implementation, returning empty string.\n Fix this warning by creating a cross platform folder for ${CMAKE_SYSTEM_NAME}:${CMAKE_SYSTEM_PROCESSOR} in ${IN_SEARCH_PATH}.")
      ELSE(${IN_MATCH_PATTERN} STREQUAL "")
        MESSAGE(WARNING "CROSS_SYSTEM_DIR failed to find cross platform implementation, returning empty string.\n Fix this warning by creating a cross platform folder for ${CMAKE_SYSTEM_NAME}:${CMAKE_SYSTEM_PROCESSOR} in ${IN_SEARCH_PATH} containing ${IN_MATCH_PATTERN}")
      ENDIF(${IN_MATCH_PATTERN} STREQUAL "")
      SET(${OUT_CROSS_SYSTEM_DIR} "" PARENT_SCOPE)
    ENDIF()
  ENDFUNCTION(CROSS_SYSTEM_DIR IN_SEARCH_PATH IN_MATCH_PATTERN OUT_CROSS_SYSTEM_DIR)
ENDIF (NOT COMMAND CROSS_SYSTEM_DIR)

IF (NOT COMMAND ADD_CROSS_SUBDIRECTORY)
  FUNCTION(add_cross_subdirectory IN_SUBDIRECTORY)
  CROSS_SYSTEM_DIR("${IN_SUBDIRECTORY}" "CMakeLists.txt" CROSS_SUBDIRECTORY)
  add_subdirectory(${CROSS_SUBDIRECTORY})
  ENDFUNCTION(add_cross_subdirectory IN_SUBDIRECTORY)
ENDIF (NOT COMMAND ADD_CROSS_SUBDIRECTORY)
