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

// Includes from this component
#include "zwave_transports_fixt.h"
#include "zwave_multi_channel_transport.h"
#include "zwave_s0_transport.h"
#include "zwave_transport_service_wrapper.h"
#include "zwave_api_transport.h"

// Generic includes
#include <stdbool.h>

// Includes from other components
#include "sl_status.h"

sl_status_t zwave_transports_init(void)
{
  sl_status_t initialization_status = SL_STATUS_OK;

  // Do not abort initialization if some transports fail.
  initialization_status |= zwave_multi_channel_transport_init();
  initialization_status |= zwave_s0_transport_init();
  // S2 inits itself with the Contiki process.
  initialization_status |= zwave_transport_service_transport_init();
  initialization_status |= zwave_api_transport_init();

  return initialization_status;
}
