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
#include "zpc_dotdot_mqtt.h"
#include "zpc_dotdot_mqtt_group_dispatch.h"
#include "dotdot_mqtt_topics_handler.h"

// Unify includes
#include "dotdot_mqtt.h"

sl_status_t zpc_dodot_mqtt_init()
{
  // Initialize the Group dispatch first.
  sl_status_t init_status = zpc_dotdot_mqtt_group_dispatch_init();
  // Initializes the ZLC attribute publishers
  init_status |= dotdot_mqtt_topics_handler_init();
  return init_status;
}