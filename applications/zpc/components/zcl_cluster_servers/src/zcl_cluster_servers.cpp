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
//Include from this component
#include "zcl_cluster_servers.h"
#include "zcl_rf_telemetry_cluster_server.h"
#include "configuration_parameter_cluster_server.h"
#include "zcl_binding_cluster_server.h"
#include "zcl_scenes_cluster_server.h"
#include "zcl_OTA_cluster_server.hpp"
#include "user_code_cluster_server.h"
#include "fan_control_cluster_server.h"

//Includes from other components
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "zpc_attribute_store_network_helper.h"
#include "attribute_store_defined_attribute_types.h"

// Interfaces
#include "zwave_controller_types.h"

///////////////////////////////////////////////////////////////////////////////
//  Init and teardown functions.
//////////////////////////////////////////////////////////////////////////////
sl_status_t zcl_cluster_servers_init()
{
  sl_status_t init_status = SL_STATUS_OK;
  init_status |= zcl_OTA_cluster_server_init();
  init_status |= zcl_rf_telemetry_cluster_server_init();
  init_status |= configuration_parameter_cluster_server_init();
  init_status |= binding_cluster_server_init();
  init_status |= zcl_scenes_cluster_server_init();
  init_status |= user_code_cluster_server_init();
  init_status |= fan_control_cluster_server_init();

  return init_status;
}

int zcl_cluster_servers_teardown()
{
  zcl_scenes_cluster_server_teardown();
  return 0;
}
