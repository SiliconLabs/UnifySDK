
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

#define NODE_TYPE_OFFSET 0x10000000

/**
 * @brief Node datastore node types
 *
 */
typedef enum {
  ZIGPC_DS_TYPE_NETWORK                   = (NODE_TYPE_OFFSET + 0x1),
  ZIGPC_DS_TYPE_DEVICE                    = (NODE_TYPE_OFFSET + 0x2),
  ZIGPC_DS_TYPE_ENDPOINT                  = (NODE_TYPE_OFFSET + 0x4),
  ZIGPC_DS_TYPE_SERVER_CLUSTER            = (NODE_TYPE_OFFSET + 0x8),
  ZIGPC_DS_TYPE_CLIENT_CLUSTER            = (NODE_TYPE_OFFSET + 0x10),
  ZIGPC_DS_TYPE_CLUSTER_ATTRIBUTE         = (NODE_TYPE_OFFSET + 0x20),
  ZIGPC_DS_TYPE_CLUSTER_GENERATED_COMMAND = (NODE_TYPE_OFFSET + 0x40),
  ZIGPC_DS_TYPE_CLUSTER_RECEIVED_COMMAND  = (NODE_TYPE_OFFSET + 0x80),
  ZIGPC_DS_TYPE_GROUP_REPORTED            = (NODE_TYPE_OFFSET + 0x100),
  ZIGPC_DS_TYPE_GROUP_DESIRED             = (NODE_TYPE_OFFSET + 0x200),
  ZIGPC_DS_TYPE_BINDING                   = (NODE_TYPE_OFFSET + 0x400),
} zigpc_datastore_node_type_t;

#ifdef __cplusplus
}
#endif
/** @} end zigpc_datastore_id */

#endif  //ZIGPC_DATASTORE_ID_H
