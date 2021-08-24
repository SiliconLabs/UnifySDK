# ##############################################################################
# Unit Testing
# ##############################################################################

# Memcheck with Valgrind if available
find_program(MEMORYCHECK_COMMAND valgrind)
set(MEMORYCHECK_COMMAND_OPTIONS "--trace-children=yes --leak-check=full --show-leak-kinds=definite,possible")
# Enable testing if SKIP_TESTING isn't set
include(CTest)
enable_testing()
