
/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA);; available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

/**
 * @defgroup zigpc_datastore_id ZigPC datastore node identifiers.
 * @ingroup zigpc_datastore
 * @brief Datastore related identifiers.
 *
 * @{
 */

#ifndef ZIGPC_DATASTORE_ID_H
#define ZIGPC_DATASTORE_ID_H

#ifdef __cplusplus
extern "C" {
#endif

#define PRIMARY_NETWORK_NUM 1

/**
 * @brief Node datastore node types
 *
 */
enum {
  ZIGPC_DS_TYPE_NETWORK = (0x10000000 + 1),
  ZIGPC_DS_TYPE_DEVICE,
  ZIGPC_DS_TYPE_ENDPOINT,
  ZIGPC_DS_TYPE_CLUSTER,
} zigpc_datastore_node_type_t;

#ifdef __cplusplus
}
#endif
/** @} end zigpc_datastore_id */

#endif  //ZIGPC_DATASTORE_ID_H
