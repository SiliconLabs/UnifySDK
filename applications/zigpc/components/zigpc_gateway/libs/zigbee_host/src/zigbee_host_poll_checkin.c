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

#include PLATFORM_HEADER
#include "app/framework/include/af.h"

#include "zap-command.h"

#include "zigbee_host.h"

EmberStatus zigbeeHostSendPollingCheckInResponse(bool startFastPolling)
{
  //In units of quarters of a second
  //by default, has a value of 32 (based on poll-control-client plugin)
  uint16_t fastPollingTimeout
    = EMBER_AF_PLUGIN_POLL_CONTROL_CLIENT_DEFAULT_FAST_POLL_TIMEOUT;

    emberAfFillCommandPollControlClusterCheckInResponse(startFastPolling,
                                                      fastPollingTimeout);

  return emberAfSendResponse();
}
