if(NOT COMPILER_OPTIONS)
  set(COMPILER_OPTIONS True)
else()
  return()
endif()

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
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_C_FLAGS
      "${CMAKE_C_FLAGS} -Wl,--unresolved-symbols=ignore-in-object-files")
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -Wl,--unresolved-symbols=ignore-in-object-files")
  add_compile_options (-fdiagnostics-color=always)
endif()

# Only add code coverage when CMAKE_GCOV is True
if(CMAKE_GCOV)
  message(STATUS "Adding GCOV flags")
  # Add gcov support
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")

  add_custom_target(
    clean-gcov
    COMMAND find ${PROJECT_BINARY_DIR} -name \\*.gcda -exec rm {} \\\\;
    COMMAND find ${PROJECT_BINARY_DIR} -name \\*.gcno -exec rm {} \\\\;
    COMMENT "Cleaning GCOV statistics")
endif()

# assume built-in pthreads on MacOS when building with gcc
if(APPLE AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_THREAD_LIBS_INIT "-lpthread")
  set(CMAKE_HAVE_THREADS_LIBRARY 1)
  set(CMAKE_USE_WIN32_THREADS_INIT 0)
  set(CMAKE_USE_PTHREADS_INIT 1)
  set(THREADS_PREFER_PTHREAD_FLAG ON)
endif()
