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
// Generic includes
#include <stdio.h>

// Includes from this component
#include "zwave_unid.h"

// Includes from other components
#include "sl_status.h"
#include "sl_log.h"

static zwave_home_id_t home_id;
#define UNID_FORMAT "zw-%08X-%04hX"
void zwave_unid_from_node_id(zwave_node_id_t node_id, unid_t unid)
{
  snprintf(unid, sizeof(unid_t), UNID_FORMAT, home_id, node_id);
}

sl_status_t zwave_unid_to_node_id(const unid_t unid, zwave_node_id_t *node_id)
{
  zwave_home_id_t unid_home;
  zwave_node_id_t unid_node;

  int i = sscanf(unid, UNID_FORMAT, &unid_home, &unid_node);
  if (i != 2) {
    return SL_STATUS_INVALID_PARAMETER;
  } else if (unid_home != home_id) {
    /// added for unit test cases, in a normal operation the returned node_id shall not be used.
    *node_id = unid_node;
    return SL_STATUS_INVALID_KEY;
  } else if (!IS_ZWAVE_NODE_ID_VALID(unid_node)) {
    return SL_STATUS_INVALID_RANGE;
  } else {
    *node_id = unid_node;
    return SL_STATUS_OK;
  }
}

sl_status_t zwave_unid_to_home_id(const unid_t unid, zwave_home_id_t *home_id)
{
  zwave_home_id_t unid_home;
  zwave_node_id_t unid_node;

  int i = sscanf(unid, UNID_FORMAT, &unid_home, &unid_node);
  if (i != 2) {
    return SL_STATUS_INVALID_PARAMETER;
  } else {
    *home_id = unid_home;
    return SL_STATUS_OK;
  }
}

void zwave_unid_set_home_id(zwave_home_id_t new_home_id)
{
  home_id = new_home_id;
}
