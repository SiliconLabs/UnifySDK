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

/**
 * @defgroup eed_attribute_store EED Attribute Store
 * @ingroup eed_components
 * @brief @ref attribute_store functions specifically for EED
 *
 * @{
 */

#ifndef EED_ATTRIBUTE_STORE_H
#define EED_ATTRIBUTE_STORE_H

#include "sl_status.h"
#include "attribute_store.h"
#include "uic_typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize attribute store specifically for EED
 *
 * @return SL_STATUS_OK if all callbacks was initialized.
 * @return SL_STATUS_FAIL if invoking any callback fails.
 */
sl_status_t eed_attribute_store_init(void);

/**
 * @brief Adds a cluster support to the given unid and endpoint
 *
 * @return SL_STATUS_OK if added successfully
 * @return SL_STATUS_FAIL if fails.
 */
sl_status_t eed_attribute_store_add_cluster_to_endpoint(dotdot_unid_t unid, dotdot_endpoint_id_t endpoint_id,
  const char* cluster_name);

/**
 * @brief Removes a cluster support to the given unid and endpoint
 *
 * @return SL_STATUS_OK if removed successfully
 * @return SL_STATUS_FAIL if fails.
 */
sl_status_t eed_attribute_store_remove_cluster_on_endpoint(dotdot_unid_t unid, dotdot_endpoint_id_t endpoint_id,
  const char* cluster_name);

// Prases and adds the cluster list entry to cluster configuration map
void cluster_config_configure(const char* unid_node, dotdot_endpoint_id_t endpoint_node, const char* cluster_list_str);

sl_status_t eed_attribute_store_command_handler_init();
sl_status_t eed_attribute_store_attribute_custom_handler_init();

#ifdef __cplusplus
}
#endif

#endif  //EED_ATTRIBUTE_STORE_H
/** @} end eed_attribute_store */
