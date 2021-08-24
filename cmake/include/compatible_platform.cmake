# Currently this is our only compatible platform. This code will evolve as other
# platforms are introduced.
if(UNIX)
  set(COMPATIBLE_PLATFORM "posix")
else()
  message(FATAL_ERROR "Platform not supported.")
endif()
