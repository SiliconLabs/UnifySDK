/* <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "aoxpc_attribute_store.h"
#include "aoxpc_attribute_store_network_helper.h"
#include "aoxpc_attribute_store_defined_attribute_types.h"

// Other components
#include "aoxpc_unid.h"

attribute_store_node_t get_aoxpc_unid_node()
{
  aoa_id_t aoxpc_unid;
  if (SL_STATUS_OK != get_aoxpc_unid(aoxpc_unid)) {
    return ATTRIBUTE_STORE_INVALID_NODE;
  }

  return aoxpc_attribute_store_network_helper_get_unid_node(aoxpc_unid);
}

attribute_store_node_t
  get_aoxpc_endpoint_id_node(dotdot_endpoint_id_t endpoint_id)
{
  aoa_id_t aoxpc_unid;
  if (SL_STATUS_OK != get_aoxpc_unid(aoxpc_unid)) {
    return ATTRIBUTE_STORE_INVALID_NODE;
  }

  return aoxpc_attribute_store_network_helper_get_endpoint_node(aoxpc_unid,
                                                                endpoint_id);
}