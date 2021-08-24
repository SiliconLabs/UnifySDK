#ifndef WORKAROUND_HPP
#define WORKAROUND_HPP

/* This is a workaround to hide the C++ includes from
 * Unity's generate_test_runner.rb
 */

#ifdef __cplusplus
#include <cstring>
#include <string>
#include "smartstart_management.hpp"
#endif
#endif
