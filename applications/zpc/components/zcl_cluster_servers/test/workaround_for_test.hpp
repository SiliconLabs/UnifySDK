#ifndef WORKAROUND_FOR_TEST_HPP
#define WORKAROUND_FOR_TEST_HPP

/* This is a workaround to hide the C++ includes from
 * Unity's generate_test_runner.rb
*/

#ifdef __cplusplus
#include <vector>
#include <utility>
#include <string>
#include <map>

#include <nlohmann/json.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

// UTF16 conversion (deprecated in C++17)
// Needed for credential data (password) per specification
#include <locale>
#include <codecvt>

#include "dotdot_mqtt_helpers.hpp"
#endif
#endif
