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

#include <string>

#include "zigpc_diagnostics_notification.hpp"

#ifndef ZIGPC_DIAGNOSTIC_METRIC_H
#define ZIGPC_DIAGNOSTIC_METRIC_H

/**
 * @brief Abstract top level metric from which all diagnostic metric derive from
 */
class zigpc_diagnostics_metric
{
  protected:
  //metric_id to which the metric will publish its resulting value
  zigpc_diagnostics_notification &on_update_complete;
  std::string metric_id;
  std::string serialized_value;

  public:
  /**
   * @brief Generic diagnostic metric constructor
   *
   * @param notif Object to call on value_update completion.
   * @param metric_id Identifier to which the metric will publish its resulting value.
   */
  zigpc_diagnostics_metric(zigpc_diagnostics_notification &notif,
                           std::string metric_id) :
    on_update_complete {notif}, metric_id {metric_id}, serialized_value {""} {};

  std::string get_metric_id(void) const
  {
    return this->metric_id;
  }

  std::string get_serialized_value(void) const
  {
    return serialized_value;
  }

  virtual void update_value(void) = 0;
};

#endif /* ZIGPC_DIAGNOSTIC_METRIC_H */
