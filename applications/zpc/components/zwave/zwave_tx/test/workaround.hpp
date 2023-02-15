#ifndef WORKAROUND_HPP
#define WORKAROUND_HPP

/* This is a workaround to hide the C++ includes from
 * Unity's generate_test_runner.rb
 */

#ifdef __cplusplus
#include <type_traits>
#include <functional>
#include <algorithm>
#include <cassert>
#include "zwave_tx_incoming_frames.hpp"
#endif
#endif
