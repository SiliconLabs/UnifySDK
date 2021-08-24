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

class zigpc_total_virtual_ram_metric : public zigpc_diagnostics_metric
{
  private:
  long long value;

  public:
  zigpc_total_virtual_ram_metric(zigpc_diagnostics_notification &notif,
                                 std::string metric_id) :
    zigpc_diagnostics_metric(notif, metric_id), value {0}
  {}

  void update_value()
  {
    struct sysinfo memInfo;

    sysinfo(&memInfo);
    long long totalVirtualMem = memInfo.totalram;
    //Avoid int overflow
    totalVirtualMem += memInfo.totalswap;
    totalVirtualMem *= memInfo.mem_unit;

    value            = totalVirtualMem;
    serialized_value = std::to_string(value);

    on_update_complete.notify(metric_id);
  }
};