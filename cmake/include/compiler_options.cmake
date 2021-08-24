# ##############################################################################
# Compiler Options
# ##############################################################################
# Setup C/CPP Standard
set(CMAKE_C_STANDARD 99) # c99
set(CMAKE_C_EXTENSIONS ON) # Enable gnu99 extensions
set(CMAKE_CXX_STANDARD 17) # C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON) # Do not allow fallback to previous C++
                                    # standards if compiler doesn't support it
set(CMAKE_CXX_EXTENSIONS ON) # Enable gnu++11 extentions

# Set compiler Flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pipe -Werror -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe -Werror -Wall")

# Only add code coverage when CMAKE_GCOV is True
if(CMAKE_GCOV)
  message(STATUS "Adding GCOV flags")
  # Add gcov support
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")

  add_custom_target(
    clean-gcov
    COMMAND find ${PROJECT_BINARY_DIR} -name \\*.gcda -exec rm {} \\\\; \\;
    COMMAND find ${PROJECT_BINARY_DIR} -name \\*.gcno -exec rm {} \\\\;
    COMMENT "Cleaning GCOV statistics")
endif()
