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

#ifndef ZWAVE_CONTROLLER_CONNECTION_INFO_H
#define ZWAVE_CONTROLLER_CONNECTION_INFO_H

#include "zwave_controller_types.h"
/**
 * @defgroup zwave_controller_connection_info Z-Wave Controller Connection Info type
 * @ingroup zwave_controller_types
 * @brief Z-Wave Controller Connection Info type definition
 **
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Encapsulation schemes
 */
typedef enum {
  /// Network Scheme
  ///
  ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME,

  /// Unencrypted
  ///
  ZWAVE_CONTROLLER_ENCAPSULATION_NONE,

  /// Security Scheme 0
  ///
  ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0,

  /// Security 2, unauthenticated
  ///
  ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_UNAUTHENTICATED,

  /// Security 2, authenticated
  ///
  ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_AUTHENTICATED,

  /// Security 2, access
  ///
  ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS,
} zwave_controller_encapsulation_scheme_t;

typedef struct {
  union {
    /// Node Id
    ///
    zwave_node_id_t node_id;
    /// Multicast group to use when sending multicast messages
    ///
    zwave_multicast_group_id_t multicast_group;
  };

  /// Endpoint Id. Set to 0 if a message is sent or received  without
  /// Multi Channel encapsulation
  /// Note that bit 7 is a multi-endpoint mask addressing. If bit 7 is set to 1
  /// the endpoint_id field MUST be interpreted as a bitmask, with bit 0
  /// representing endpoint 1, bit 1, representing endpoint 2, ...,
  /// bit 6 representing endpoint 7.
  zwave_endpoint_id_t endpoint_id;

  /// This flag is set to true if should be sent as a multicast frame
  /// S2 Multicasts or unencrypted broadcasts must not use this bit and
  /// simply set the remote.node_id to ZWAVE_BROADCAST_NODE_ID.
  bool is_multicast;
} zwave_controller_endpoint_t;

/**
 * @brief Structure holding information about the source and destination
 * when transmitting and receiving Z-Wave frames.
 */
typedef struct zwave_controller_connection_info {
  /// Local end of this connection, ie this device
  ///
  zwave_controller_endpoint_t local;

  /// remote end of this connection, ie not this device.
  ///
  zwave_controller_endpoint_t remote;

  /// The encapsulation used by the sender. When transmitting
  /// the sender is this device.
  ///
  zwave_controller_encapsulation_scheme_t encapsulation;
} zwave_controller_connection_info_t;

#ifdef __cplusplus
}
#endif
/** @} end zwave_controller_connection_info */

#endif  //ZWAVE_CONTROLLER_CONNECTION_INFO_H