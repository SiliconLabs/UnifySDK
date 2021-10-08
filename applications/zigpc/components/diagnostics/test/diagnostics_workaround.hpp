#ifndef WORKAROUND_DIAGNOSTIC_HPP
#define WORKAROUND_DIAGNOSTIC_HPP

/* This is a workaround to hide the C++ includes from
 * Unity's generate_test_runner.rb
 */

#ifdef __cplusplus
#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include "zigpc_diagnostics_mqtt_helper.hpp"
#include "zigpc_diagnostics_manager.hpp"
#include "zigpc_diagnostics_notification.hpp"
#include "zigpc_diagnostics_metric.hpp"

#include "zigpc_uptime.hpp"
#include "zigpc_total_virtual_ram.hpp"
#include "zigpc_used_virtual_ram.hpp"
#include "zigpc_total_physical_ram.hpp"
#include "zigpc_used_physical_ram.hpp"
#include "zigpc_cpu_usage.hpp"
#include "zigpc_example_metric.hpp"
#include "zigpc_counter_plugin_metric.hpp"
#endif
#endif