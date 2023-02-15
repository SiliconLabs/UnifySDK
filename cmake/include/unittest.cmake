# ##############################################################################
# Unit Testing
# ##############################################################################

# Memcheck with Valgrind if available
find_program(MEMORYCHECK_COMMAND valgrind)
set(MEMORYCHECK_COMMAND_OPTIONS
    "--trace-children=yes --leak-check=full --show-leak-kinds=definite,possible"
)

include(CTest)
set(CTEST_OUTPUT_ON_FAILURE ON)
