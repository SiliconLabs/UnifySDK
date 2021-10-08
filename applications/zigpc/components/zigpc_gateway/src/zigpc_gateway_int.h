/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#ifndef ZIGPC_GATEWAY_INT_H
#define ZIGPC_GATEWAY_INT_H

// ZigPC includes
#include <zigpc_common_zigbee.h>
#include <zigpc_config.h>

/* Z3gateway includes */
#include "z3gateway.h"

#include "zigpc_gateway_notify.h"

#ifdef __cplusplus
extern "C" {
#endif

static const char LOG_TAG[] = "zigpc_gateway";

/* Unique keys to use in datastore */
#define ZIGPC_GATEWAY_STORE_DEVICE_TABLE_ARR_KEY  "zigpc_gsdk_dt"
#define ZIGPC_GATEWAY_STORE_DEVICE_TABLE_SIZE_KEY "zigpc_gsdk_dt_size"
#define ZIGPC_GATEWAY_STORE_DEVICE_TABLE_CLUSTER_ARR_KEY \
  "zigpc_gsdk_dt_clusters"

#define ZIGPC_GATEWAY_NOTIFICATIONS_MAX (10)

enum zigpc_gateway_dispatch_type {
  ZIGPC_GW_DISPATCH_MIN_VAL = 0,
  ZIGPC_GW_DISPATCH_NETWORK_INIT,
  ZIGPC_GW_DISPATCH_PERMIT_JOINS,
  ZIGPC_GW_DISPATCH_ADD_NODE_INSTALL_CODE,
  ZIGPC_GW_DISPATCH_INTERVIEW_NODE,
  ZIGPC_GW_DISPATCH_REMOVE_NODE,
  ZIGPC_GW_DISPATCH_MAX_VAL,
};

typedef struct {
  bool send_as_multicast;
  zigbee_multicast_id_t multicast_id;
  zigbee_eui64_t eui64;
  zigbee_endpoint_id_t endpoint_id;
  zcl_cluster_id_t cluster_id;
  zcl_frame_t frame;
} zigpc_gateway_dispatch_zcl_frame_t;

typedef struct {
  zigbee_eui64_t eui64;
  uint8_t endpoint_id;
  zcl_cluster_id_t cluster_id;
  uint8_t *report_record;
  unsigned int record_size;
} zigpc_gateway_dispatch_init_report_t;

typedef struct {
  zigbee_eui64_t eui64;
  uint8_t endpoint_id;
  zcl_cluster_id_t cluster_id;
} zigpc_gateway_dispatch_bind_request_t;

struct zigpc_gateway_dispatch_add_node {
  zigbee_eui64_t eui64;
  zigbee_install_code_t install_code;
  uint8_t install_code_length;
};

/**
 * @brief Data for permitting joins to the Zigbee network.
 *
 */
typedef struct {
  bool enable;
} zigpc_gateway_dispatch_permit_joins_t;

/**
 * @brief Data for processing node removal.
 *
 */
typedef struct {
  zigbee_eui64_t eui64;
} zigpc_gateway_dispatch_remove_node_t;

/**
 * @brief Data for processing node interview
 *
 */
typedef struct {
  zigbee_eui64_t eui64;
} zigpc_gateway_dispatch_interview_node_t;

typedef struct {
    char filename[ZCL_DEFAULT_STR_LENGTH];
    unsigned int filename_size;
} zigpc_gateway_dispatch_add_ota_image_t;

/**
 * @brief Data container for dispatch events
 *
 */
struct zigpc_gateway_dispatch_event {
  enum zigpc_gateway_dispatch_type type;
  union {
    struct zigpc_gateway_dispatch_add_node add_node;
    zigpc_gateway_dispatch_remove_node_t remove_node;
    zigpc_gateway_dispatch_interview_node_t interview_node;
    zigpc_gateway_dispatch_permit_joins_t permit_joins;
  };
};

extern struct z3gatewayCallbacks zigpc_gateway_z3gateway_callbacks;

/**
 * Internal API for notify system
 */

/**
 * @brief Reference to Registered Gateway Observers
 *
 */
extern struct zigpc_observable zigpc_gateway_observable;

/**
 * @brief Clear registered observers
 *
 */
sl_status_t zigpc_gateway_reset_observers(void);

/*
 * Internal API for dispatch system
 */

/**
 * @brief Forward events received to z3gateway
 *
 * @param event             Dispatch event structure
 */
sl_status_t zigpc_gateway_dispatch(struct zigpc_gateway_dispatch_event *event);

/**
 * @brief Return string representation of dispatch type
 *
 * @param type              Dispatch type
 * @return const char*      String representation of dispatch type.
 */
const char *
  zigpc_gateway_dispatch_name(const enum zigpc_gateway_dispatch_type type);

/**
 * @brief zigpc_gateway_dispatch_zcl_frame
 * Dispatches a ZCL command to be processed by the z3gateway, and sent out
 * on to the zigbee network
 *
 * @param zcl_data -a structure containing all the information required to
 *                  send a ZCL command
 *
 * @return the status if the command was successfully parsed and sent
**/
sl_status_t zigpc_gateway_dispatch_zcl_frame(
  const zigpc_gateway_dispatch_zcl_frame_t *zcl_data);

/**
 * @brief zigpc_gateway_dispatch_init_reports
 * Dispatches a ZCL configure reports command to a given endpoint
 *
 * @param report_data -a structure containing all the information required to
 *                  send a ZCL configure reports command
 *
 * @return the status if the command was successfully parsed and sent
**/
sl_status_t zigpc_gateway_dispatch_init_reports(
  zigpc_gateway_dispatch_init_report_t *report_data);

/**
 * @brief zigpc_gateway_dispatch_config_binding
 * Dispatches a Zigbee message to request to configure a binding on an endpoint
 *
 * @param binding_data -a structure containing all the information required to
 *                  send a zigbee bind request
 *
 * @return the status if the command was successfully parsed and sent
**/
sl_status_t zigpc_gateway_dispatch_config_binding(
  zigpc_gateway_dispatch_bind_request_t *binding_data);

/**
 * @brief Retrieve persisted information related to Z3Gateway and ZigPC Gateway
 * from the datastore. This includes:
 * - GSDK device-table plugin state: first the number of entries stored in the
 * datastore is retrieved. Then, each datastore entry (and the associated
 * cluster list) is loaded and imported using the z3gateway device table import
 * entry API.
 *
 * @return sl_status_t SL_STATUS_OK if all information was loaded from datastore
 * successfully, error status if not.
 */
sl_status_t zigpc_gateway_load_from_datastore(void);

/**
 * @brief Persist information related to Z3Gateway and ZigPC Gateway to the
 * datastore. This includes:
 * - GSDK device-table plugin state: first the number of entries used by the
 * device table is retrieved. Then, each entry (and the associated cluster
 * list) is exported using the z3gateway API and saved in the datastore as
 * three separate info types: device_table size, device_table_entry[i], and
 * device_table_entry[i].clusterList.
 *
 * @return sl_status_t SL_STATUS_OK if all information was store to datastore
 * successfully, error status if not.
 */
sl_status_t zigpc_gateway_save_to_datastore(void);

sl_status_t zigpc_gateway_dispatch_ota_image(
        const zigpc_gateway_dispatch_add_ota_image_t *ota_image_data);

/**
 * @brief zigpc_gateway_unload_sleepy_messages
 * A helper function to unload all ZCL frames stored for a given
 * sleepy device, determined by its eui64
 *
 * @param eui64 - the eui64 of a given sleepy end device
 *
 * @return SL_STATUS_OK if able to successfully find and send the
 * ZCL frames associated with the eui64. SL_STATUS_FAIL otherwise
**/
sl_status_t zigpc_gateway_unload_sleepy_messages(const zigbee_eui64_t eui64);
#ifdef __cplusplus
}
#endif

#endif  // ZIGPC_GATEWAY_INT_H
