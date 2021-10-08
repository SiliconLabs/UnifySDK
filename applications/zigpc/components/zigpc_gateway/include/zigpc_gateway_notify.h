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

/**
 * @file zigpc_gateway_notify.h
 * @defgroup  zigpc_gateway_notify ZigPC Observer API
 * @ingroup zigpc_gateway
 * @brief Observer API for getting events from zigpc_gateway
 *
 * In order to listen to events from Z3Gateway, ZigPC Gateway allows components
 * to register observer functions that receive an event type and associated data
 *
 * @{
 */

#ifndef ZIGPC_GATEWAY_NOTIFY_H
#define ZIGPC_GATEWAY_NOTIFY_H

/* Shared component includes */
#include "sl_status.h"

#include "zcl_definitions.h"
#include "zigpc_common_zigbee.h"
#include "zigpc_common_observable.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The following are the types of events dispatched to observers of
 * zigpc_gateway.
 *
 */
enum zigpc_gateway_notify_event {
  /**
   * @brief Used when informing that the Gateway network has initialized.
   * Users should wait for this event before performing operations
   * on the Zigbee network. If this callback is not invoked within an
   * appropriate delay, it is safe to assume that network initialization has
   * failed. See zigpc_gateway_on_network_init for associated data.
   *
   */
  ZIGPC_GATEWAY_NOTIFY_NETWORK_INIT = 0,

  /**
   * @brief Used when informing that a node has been added  to the Gateway
   * network. See zigpc_gateway_on_node_add for associated data.
   *
   */
  ZIGPC_GATEWAY_NOTIFY_NODE_ADD_START,
  ZIGPC_GATEWAY_NOTIFY_NODE_ADD_COMPLETE,

  /**
   * @brief Used when informing that a node on the Gateway network has been
   * removed. See zigpc_gateway_on_node_removed_t for associated data.
   *
   */
  ZIGPC_GATEWAY_NOTIFY_NODE_REMOVED,

  /**
   * @brief Used when informing that a node on the Gateway network has been
   * interviewed for the number of endpoints supported. See
   * zigpc_gateway_on_node_discovered_t for associated data.
   *
   * NOTE: This event should be received before the endpoint discovery event
   * for a particular device.
   *
   */
  ZIGPC_GATEWAY_NOTIFY_NODE_DISCOVERED,

  /**
   * @brief Used when informing that a node on the Gateway network has been
   * interviewed for cluster information for a particular endpoint. See
   * zigpc_gateway_on_node_endpoint_discovered_t for associated data.
   *
   * NOTE: Multiple events of this type should be sent based on the number of
   * endpoints supported by the node on the network. This event should only be
   * received after a node discovery event is received.
   *
   */
  ZIGPC_GATEWAY_NOTIFY_NODE_ENDPOINT_DISCOVERED,

  /**
   * @brief Used when an a device endpoint has sent a cluster attribute report.
   * See zigpc_gateway_on_attribute_reported_t for associated data.
   *
   */
  ZIGPC_GATEWAY_NOTIFY_REPORTED_ATTRIBUTE,

  /**
   * @brief Used when informing that a node on the Gateway network has sent
   * a ZCL global read attribute response containing the latest attribute
   * information for a ZCL cluster. See
   * zigpc_gateway_on_read_attribute_response_t for associated data.
   *
   */
  ZIGPC_GATEWAY_NOTIFY_READ_ATTRIBUTE_RESPONSE,

  /**
   * @brief Used when a node on the Gateway has received a ZCL command frame
   * from a node on the network. see zigpc_gateway_on_command_received_t for
   * associated data.
   *
   */
  ZIGPC_GATEWAY_NOTIFY_ZCL_COMMAND_RECEIVED,

  /**
   * @brief Used when a device has started an OTA update process. see
   * zigpc_gateway_on_ota_started_t for associated data.
   *
   */
  ZIGPC_GATEWAY_NOTIFY_OTA_UPDATE_STARTED,

  /**
   * @brief Used when a device has complete an OTA update. see
   * zigpc_gateway_on_ota_completed_t for associated data.
   *
   */
  ZIGPC_GATEWAY_NOTIFY_OTA_UPDATE_COMPLETED,

  ZIGPC_GATEWAY_NOTIFY_MAX,
};

/**
 * @brief Event data passed after successful Gateway network initialization.
 *
 */
struct zigpc_gateway_on_network_init {
  zigbee_eui64_t zigpc_eui64;                 /**< Gateway EUI64 */
  zigbee_panid_t zigpc_panid;                 /**< Network PAN ID */
  zigbee_ext_panid_t zigpc_ext_panid;         /**< Network extended Pan ID */
  zigbee_radio_channel_t zigpc_radio_channel; /**<  Network radio channel */
};

/**
 * @brief Event data passed after successful node addition to the Gateway
 * network.
 *
 */
typedef struct zigpc_gateway_on_node_add {
  zigbee_eui64_t eui64; /**< Joined device EUI64 in big endian */
} zigpc_gateway_on_node_add_t;

/**
 * @brief Event data passed when receiving node removal complete event.
 *
 */
typedef struct {
  zigbee_eui64_t eui64; /**< Removed device EUI64 in big endian */
} zigpc_gateway_on_node_removed_t;

/**
 * @brief Event data passed when receiving count of endpoint discovery events
 * to wait for a particular node on the network.
 *
 */
typedef struct {
  zigbee_eui64_t eui64;   /**< Discovered device EUI64 in big endian */
  uint8_t endpoint_count; /**< Number of endpoints supported by device */
} zigpc_gateway_on_node_discovered_t;

/**
 * @brief Event data passed when receiving cluster information for a probed
 * endpoint in a node on the network.
 *
 */
typedef struct {
  zigbee_eui64_t eui64;       /**< Discovered device EUI64 in big endian */
  zigbee_endpoint_t endpoint; /**< Single Endpoint information of the device */
} zigpc_gateway_on_node_endpoint_discovered_t;

typedef struct {
  zigbee_eui64_t eui64;
  zigbee_endpoint_id_t endpoint_id;
  zcl_cluster_id_t cluster_id;
  zcl_frame_t frame; /**< ZCL frame buffer */
} zigpc_gateway_on_attribute_reported_t;

/**
 * @brief Event data passed when receiving a ZCL global read attributes
 * response from a device on the network.
 *
 */
typedef struct {
  zigbee_eui64_t eui64;             /**< Device EUI64 in big endian */
  zigbee_endpoint_id_t endpoint_id; /**< Endpoint idenfitier */
  zcl_cluster_id_t cluster_id;      /**< ZCL cluster identifier */
  zcl_frame_t frame;                /**< ZCL frame buffer */
} zigpc_gateway_on_read_attribute_response_t;

/**
 * @brief Event data passed when receiving a ZCL cluster command payload from
 * a device on the network.
 *
 */
typedef struct {
  zigbee_eui64_t eui64;             /**< Device EUI64 in big endian */
  zigbee_endpoint_id_t endpoint_id; /**< Endpoint idenfitier */
  zcl_cluster_id_t cluster_id;      /**< ZCL cluster identifier */
  zcl_command_id_t command_id;      /**< ZCL cluster command identifier */
  bool from_server_to_client;       /**< Indicate command direction */
  size_t frame_payload_offset;      /**< Start index of command payload */
  zigpc_zcl_status_t return_status; /**< Return status of command processing */
  zcl_frame_t frame;                /**< ZCL frame buffer */
} zigpc_gateway_on_command_received_t;

/**
 * @brief Event data passed when receiving an OTA update started message for a
 * device.
 *
 */
typedef struct {
  zigbee_eui64_t eui64;      /**< Device EUI64 in big endian */
  uint16_t manufacturer_id;  /**< Image manufacturer ID */
  uint16_t image_type_id;    /**< Image type ID */
  uint32_t firmware_version; /**< Image firmware version */
} zigpc_gateway_on_ota_started_t;

/**
 * @brief Event data passed when receiving an OTA update complete message for
 * a device.
 *
 */
typedef struct {
  zigbee_eui64_t eui64;      /**< Device EUI64 in big endian */
  uint16_t manufacturer_id;  /**< Image manufacturer ID */
  uint16_t image_type_id;    /**< Image type ID */
  uint32_t firmware_version; /**< Image firmware version */
  zigpc_zcl_status_t status; /**< Complete status */
} zigpc_gateway_on_ota_completed_t;

/**
 * @brief Register an observer to be informed of Z3Gateway updates.
 * If the observer is already registered, nothing will be done.
 *
 * @param event           ZigPC Gateway notify event type
 * @param callback        Callback to register with event
 * @return sl_status_t    SL_STATUS_OK on success
 */
sl_status_t
  zigpc_gateway_register_observer(enum zigpc_gateway_notify_event event,
                                  zigpc_observer_callback_t callback);

/**
 * @brief Unregister an observer from Z3Gateway updates.
 * If the observer is not found, nothing will be done.
 *
 * @param event           ZigPC Gateway notify event type
 * @param callback        Callback to un-register with event
 * @return sl_status_t    SL_STATUS_OK on removal
 */
sl_status_t
  zigpc_gateway_unregister_observer(enum zigpc_gateway_notify_event event,
                                    zigpc_observer_callback_t callback);

#ifdef __cplusplus
}
#endif

#endif  //ZIGPC_GATEWAY_NOTIFY_H

/** @} end zigpc_gateway_notify */
