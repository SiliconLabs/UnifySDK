/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "eed_attribute_store_helper.h"
#include "eed_attribute_store_attribute_types.h"
#include "eed_attribute_store.h"

// Generic includes
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// Includes from other components
#include "attribute_store_helper.h"
#include "sl_log.h"
#include "sl_status.h"
#include "unify_dotdot_attribute_store.h"
#include "unify_dotdot_defined_attribute_types.h"
#include "unify_dotdot_attribute_store_node_state.h"

#define LOG_TAG "eed_attribute_store"

attribute_store_node_t eed_attribute_store_get_unid_node(dotdot_unid_t unid)
{
  char unid_value[MAXIMUM_UNID_SIZE] = {};
  attribute_store_node_t root        = attribute_store_get_root();
  for (size_t i = 0;
       i < attribute_store_get_node_child_count_by_type(root, ATTRIBUTE_UNID);
       i++) {
    attribute_store_node_t unid_node
      = attribute_store_get_node_child_by_type(root, ATTRIBUTE_UNID, i);
    attribute_store_get_reported_string(unid_node,
                                        unid_value,
                                        MAXIMUM_UNID_SIZE);

    if (strcmp(unid, unid_value) == 0) {
      return unid_node;
    }
  }
  return ATTRIBUTE_STORE_INVALID_NODE;
}

attribute_store_node_t
  eed_attribute_store_get_endpoint_node(dotdot_unid_t unid,
                                        dotdot_endpoint_id_t endpoint_id)
{
  attribute_store_node_t unid_node = eed_attribute_store_get_unid_node(unid);
  return attribute_store_get_node_child_by_value(unid_node,
                                                 ATTRIBUTE_ENDPOINT_ID,
                                                 REPORTED_ATTRIBUTE,
                                                 &endpoint_id,
                                                 sizeof(endpoint_id),
                                                 0);
}

sl_status_t eed_attribute_store_get_unid_from_node(attribute_store_node_t node,
                                                   char *unid)
{
  attribute_store_node_t unid_node;
  if (attribute_store_get_node_type(node) == ATTRIBUTE_UNID) {
    unid_node = node;
  } else {
    unid_node
      = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_UNID);
  }
  return attribute_store_get_reported_string(unid_node,
                                             unid,
                                             MAXIMUM_UNID_SIZE);
}

sl_status_t eed_attribute_store_get_endpoint_from_node(
  attribute_store_node_t node, char *unid, dotdot_endpoint_id_t *endpoint_id)
{
  attribute_store_node_t endpoint_node;
  if (attribute_store_get_node_type(node) == ATTRIBUTE_ENDPOINT_ID) {
    endpoint_node = node;
  } else {
    endpoint_node
      = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID);
  }
  sl_status_t status = SL_STATUS_OK;
  status |= attribute_store_get_reported(endpoint_node,
                                         endpoint_id,
                                         sizeof(dotdot_endpoint_id_t));
  status |= eed_attribute_store_get_unid_from_node(node, unid);
  return status;
}