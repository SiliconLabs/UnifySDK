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
#include "zigpc_diagnostics_metric.hpp"

#include "sys/types.h"
#include "sys/sysinfo.h"

enum cpu_load_config { one_minute = 0, five_minute = 1 , fifthteen_minute = 2 };

class zigpc_cpu_usage : public zigpc_diagnostics_metric
{
  private:
  float value;
  cpu_load_config config;


  public:
  zigpc_cpu_usage(zigpc_diagnostics_notification &notif,
                      std::string metric_id, cpu_load_config config) :
    zigpc_diagnostics_metric(notif, metric_id), value {0}, config(config)
  {}

  void update_value()
  {
    struct sysinfo memInfo;

    sysinfo(&memInfo);

    float f_load       = 1.f / (1 << SI_LOAD_SHIFT);
    float cpu_1_m_load = memInfo.loads[config];

    value            = cpu_1_m_load * f_load * 100 / get_nprocs();
    serialized_value = std::to_string(value);

    on_update_complete.notify(metric_id);
  }
};