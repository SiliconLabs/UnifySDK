#ifndef WORKAROUND_FOR_TEST_HPP
#define WORKAROUND_FOR_TEST_HPP

/* This is a workaround to hide the C++ includes from
 * Unity's generate_test_runner.rb
*/

#ifdef __cplusplus
#include <filesystem>
#include <map>
#endif
#endif
