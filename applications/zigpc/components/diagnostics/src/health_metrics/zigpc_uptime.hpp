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
#ifndef ZIGPC_DIAGNOSTIC_UPTIME_H
#define ZIGPC_DIAGNOSTIC_UPTIME_H
#include <time.h>
#include <stdio.h>
#include <linux/unistd.h>
#include <errno.h>
#include <linux/kernel.h>
#include <sys/sysinfo.h>
#include "sl_log.h"

#include "zigpc_diagnostics_metric.hpp"

class zigpc_uptime_metric : public zigpc_diagnostics_metric
{
  private:
  long value;

  public:
  zigpc_uptime_metric(zigpc_diagnostics_notification &notif,
                      std::string metric_id) :
    zigpc_diagnostics_metric(notif, metric_id), value {0}
  {}
  ~zigpc_uptime_metric() {}
  void update_value()
  {
    struct sysinfo s_info;
    int error = sysinfo(&s_info);
    if (error != 0) {
      sl_log_error("zigpc uptime metric",
                   "Failed to read uptime code error = %d\n",
                   error);
    }

    value            = s_info.uptime;
    serialized_value = std::to_string(value);

    on_update_complete.notify(metric_id);
  }
};
#endif