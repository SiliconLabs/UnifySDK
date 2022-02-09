/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#include <array>
#include <iomanip>
#include <string>
#include <sstream>

// System headers
#include <sys/types.h>
#include <sys/sysinfo.h>

// Unify includes
#include <sl_status.h>

// Component includes
#include "zigpc_diagnostics_metric.hpp"

class zigpc_cpu_load_metric : public zigpc_diagnostics_metric
{
  public:
  /**
   * @brief CPU Load average types
   *
   */
  enum class load_type : uint8_t { AVG_1_MIN, AVG_5_MIN, AVG_15_MIN };

  private:
  static constexpr char LOG_TAG[] = "diagnostics_metric_cpu_load";
  static constexpr int LOAD_COUNT = 3U;
  static constexpr std::array<std::string_view, LOAD_COUNT> LOAD_LABELS
    = {"1Min", "5Min", "15Min"};

  std::array<double, LOAD_COUNT> load_list = {};

  public:
  zigpc_cpu_load_metric(zigpc_diagnostics_notification &notif,
                        std::string metric_id) :
    zigpc_diagnostics_metric(notif, metric_id)
  {}

  double get_value(load_type type) const
  {
    switch (type) {
      case load_type::AVG_1_MIN:
        return load_list[0U];
      case load_type::AVG_5_MIN:
        return load_list[1U];
      case load_type::AVG_15_MIN:
        return load_list[2U];
      default:
        return -1.0;
    }
  }

  void update_value()
  {
    int err = getloadavg(load_list.data(), LOAD_COUNT);
    if (err < LOAD_COUNT) {
      sl_log_error(LOG_TAG,
                   "Failed to retrieve sysinfo. err: %d, errono: %d",
                   err,
                   errno);
      return;
    }

    std::stringstream ss;
    ss << "{";
    char sep = ' ';
    for (size_t i = 0U; i < LOAD_COUNT; i++) {
      ss << sep << '"' << LOAD_LABELS[i] << "\":";
      ss << std::fixed << std::setprecision(2) << load_list[i];
      sep = ',';
    }
    ss << "}";

    serialized_value = std::move(ss.str());

    on_update_complete.notify(metric_id);
  }
};