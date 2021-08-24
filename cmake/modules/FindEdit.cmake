# Tries to find libedit headers and libraries
#
# Usage of this module as follows:
#
# find_package(Edit)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
# * EDIT_ROOT_DIR   Set this variable to the root installation of editline if
#   the module has problems finding the proper installation path.
#
# Variables defined by this module:
#
# * EDIT_FOUND System has Editline libs/headers
# * EDIT_LIBRARIES The Editline libraries
# * EDIT_INCLUDE_DIR The location of Editline headers

find_path(
  EDIT_INCLUDE_DIR
  NAMES histedit.h
  HINTS ${EDIT_ROOT_DIR}/include)

find_library(
  EDIT_LIBRARIES
  NAMES edit
  HINTS ${EDIT_ROOT_DIR}/lib)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Edit DEFAULT_MSG EDIT_LIBRARIES
                                  EDIT_INCLUDE_DIR)

mark_as_advanced(EDIT_ROOT_DIR EDIT_LIBRARIES EDIT_INCLUDE_DIR)
