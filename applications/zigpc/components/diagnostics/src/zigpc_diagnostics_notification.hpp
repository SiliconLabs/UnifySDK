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

#ifndef ZIGPC_DIAGNOSTIC_NOTIFICATION_H
#define ZIGPC_DIAGNOSTIC_NOTIFICATION_H

#include <string>

class zigpc_diagnostics_notification
{
  public:
  virtual void notify(std::string metric_id) {};
};

#endif /* ZIGPC_DIAGNOSTIC_NOTIFICATION_H */