#ifndef WORKAROUND_HPP
#define WORKAROUND_HPP

/* This is a workaround to hide the C++ includes from
 * Unity's generate_test_runner.rb
 */

#ifdef __cplusplus
#include "json_helpers.hpp"
#include <vector>
#include <utility>
#include <string>
#include <nlohmann/json.hpp>
#endif
#endif