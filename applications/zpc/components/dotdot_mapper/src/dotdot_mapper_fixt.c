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

// Includes from other component
#include "sl_status.h"

// Includes from this component
#include "dotdot_mapper_fixt.h"
#include "basic_cluster_mapper.h"
#include "on_off_cluster_mapper.h"
#include "level_cluster_mapper.h"
#include "color_control_cluster_mapper.h"
#include "thermostat_cluster_mapper.h"
#include "sl_status.h"
#include "door_lock_cluster_mapper.h"
#include "identify_cluster_mapper.h"
#include "binding_cluster_mapper.h"

sl_status_t dotdot_mapper_init()
{
  // Initializes all clusters individually.
  bool init_status = true;

  // Basic Cluster
  init_status &= basic_cluster_mapper_init();
  init_status &= on_off_cluster_mapper_init();
  init_status &= level_cluster_mapper_init();
  init_status &= color_control_cluster_mapper_init();
  init_status &= thermostat_cluster_mapper_init();
  init_status &= door_lock_cluster_mapper_init();
  init_status &= identify_cluster_mapper_init();
  init_status &= binding_cluster_mapper_init();

  return (init_status == true) ? SL_STATUS_OK : SL_STATUS_FAIL;
}

int dotdot_mapper_teardown()
{
  basic_cluster_mapper_teardown();
  on_off_cluster_mapper_teardown();
  level_cluster_mapper_teardown();
  thermostat_cluster_mapper_teardown();
  door_lock_cluster_mapper_teardown();
  return 0;
}
