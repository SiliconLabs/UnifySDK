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
#include "zpc_attribute_store_test_helper.h"
#include "attribute_store_fixt.h"
#include "attribute_store.h"
#include "attribute_store_defined_attribute_types.h"
#include "attribute.hpp"
#include "zwave_unid.h"
#include "zwave_controller_types.h"

using namespace attribute_store;

extern "C" {
const zwave_home_id_t home_id               = 0xcafecafe;
const zwave_node_id_t zpc_node_id           = 1;
const zwave_node_id_t zpc_endpoint_id       = 0;
const zwave_node_id_t node_id               = 4;
const zwave_endpoint_id_t endpoint_id       = 3;
unid_t zpc_unid                             = "zw-CAFECAFE-0001";
unid_t supporting_node_unid                 = "zw-CAFECAFE-0004";
attribute_store_node_t home_id_node         = ATTRIBUTE_STORE_INVALID_NODE;
attribute_store_node_t zpc_node_id_node     = ATTRIBUTE_STORE_INVALID_NODE;
attribute_store_node_t node_id_node         = ATTRIBUTE_STORE_INVALID_NODE;
attribute_store_node_t endpoint_id_node     = ATTRIBUTE_STORE_INVALID_NODE;
attribute_store_node_t zpc_endpoint_id_node = ATTRIBUTE_STORE_INVALID_NODE;

void zpc_attribute_store_test_helper_create_network()
{
  // Configure the UNID module to know our UNID.
  zwave_unid_set_home_id(home_id);
  zwave_unid_from_node_id(zpc_node_id, zpc_unid);
  zwave_unid_from_node_id(node_id, supporting_node_unid);

  // Make sure to start from scratch
  attribute_store_delete_node(attribute_store_get_root());
  // HomeID
  home_id_node
    = attribute_store_add_node(ATTRIBUTE_HOME_ID, attribute_store_get_root());
  attribute_store_set_reported(home_id_node, &home_id, sizeof(zwave_home_id_t));

  // NodeID 1 (it's the ZPC)
  zpc_node_id_node = attribute_store_add_node(ATTRIBUTE_NODE_ID, home_id_node);
  attribute_store_set_reported(zpc_node_id_node,
                               &zpc_node_id,
                               sizeof(zpc_node_id));

  zpc_endpoint_id_node
    = attribute_store_add_node(ATTRIBUTE_ENDPOINT_ID, zpc_node_id_node);
  attribute_store_set_reported(zpc_endpoint_id_node,
                               &zpc_endpoint_id,
                               sizeof(zwave_endpoint_id_t));

  // NodeID 4 (Supporting node)
  node_id_node = attribute_store_add_node(ATTRIBUTE_NODE_ID, home_id_node);
  attribute_store_set_reported(node_id_node, &node_id, sizeof(node_id));

  endpoint_id_node
    = attribute_store_add_node(ATTRIBUTE_ENDPOINT_ID, node_id_node);
  attribute_store_set_reported(endpoint_id_node,
                               &endpoint_id,
                               sizeof(endpoint_id));
}
}