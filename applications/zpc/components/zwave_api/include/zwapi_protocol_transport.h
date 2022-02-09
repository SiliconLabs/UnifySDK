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

/**
 * @file zwapi_protocol_transport.h
 * @brief Z-Wave API for transport application layer interface.
 */

#ifndef ZWAPI_PROTOCOL_TRANSPORT_H
#define ZWAPI_PROTOCOL_TRANSPORT_H

#include <stdint.h>
#include <stdbool.h>
#include "sl_status.h"
#include "zwave_node_id_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup ZWAPI_TRANSPORT Transport API
 * @ingroup zwave_api
 * @brief Functions used to send data to Z-Wave nodes.
 *
 * Functions in this module are used to transmit commands to Z-Wave nodes using
 * single-cast and multi-cast.
 * @{
 */

/**
 * The Z Wave transport layer controls transfer of data between Z Wave nodes
 * including retransmission, frame check, and acknowledgment. The Z Wave
 * transport interface includes functions for transfer of data to other Z Wave
 * nodes. Application data received from other nodes is handed over to the
 * application via the @ref application_command_handler_function function. The
 * ZW_MAX_NODES define defines the maximum of nodes possible in a Z Wave
 * network.
 */

/// Max. number of parameters
#define NODEPARM_MAX 35

/// @name Transmit frame option flags
///@{
///< request acknowledge from destination node
#define TRANSMIT_OPTION_ACK 0x01
///< transmit at low output power level (1/3 of normal RF range)
#define TRANSMIT_OPTION_LOW_POWER 0x02
///< The multicast frame should be send as a broadcast
#define TRANSMIT_OPTION_MULTICAST_AS_BROADCAST 0x02
/// request retransmission via repeater nodes
#define TRANSMIT_OPTION_AUTO_ROUTE 0x04
/// do not use response route - Even if available
#define TRANSMIT_OPTION_NO_ROUTE 0x10
/// Use explore frame if needed
#define TRANSMIT_OPTION_EXPLORE 0x20
/// The the frame as a multicast frame
#define TRANSMIT_OPTION_MULTICAST 0x40
///@}

/// @name Transmit frame option flag which are valid when sending explore frames
///@{
#define TRANSMIT_EXPLORE_OPTION_ACK       TRANSMIT_OPTION_ACK
#define TRANSMIT_EXPLORE_OPTION_LOW_POWER TRANSMIT_OPTION_LOW_POWER
///@}

/// @name Allow Transport Service segmentation of long messages
///@{
#define TRANSMIT_OPTION_2_TRANSPORT_SERVICE 0x01
#define TRANSMIT_OPTION_2_FOLLOWUP          0x08
///@}

/// @name Received frame status flags
///@{
/// A response route is locked by the application
#define RECEIVE_STATUS_ROUTED_BUSY 0x01
/// Received at low output power level, this must have the same value as
/// TRANSMIT_OPTION_LOW_POWER
#define RECEIVE_STATUS_LOW_POWER 0x02
/// Mask for masking out the received frametype bits
#define RECEIVE_STATUS_TYPE_MASK 0x0C
/// Received frame is singlecast frame (rxOptions == xxxx00xx)
#define RECEIVE_STATUS_TYPE_SINGLE 0x00
/// Received frame is broadcast frame  (rxOptions == xxxx01xx)
#define RECEIVE_STATUS_TYPE_BROAD 0x04
/// Received frame is multicast frame (rxOptions == xxxx10xx)
#define RECEIVE_STATUS_TYPE_MULTI 0x08
/// Received frame is an explore frame (rxOptions == xxx1xxxx). Only TYPE_BROAD
/// can be active at the same time as TYPE_EXPLORE
#define RECEIVE_STATUS_TYPE_EXPLORE 0x10
/// Received frame is not send to me (rxOptions == x1xxxxxx). Useful only in
/// promiscuous mode
#define RECEIVE_STATUS_FOREIGN_FRAME 0x40
/// Received frame is send on another network (rxOptions == 1xxxxxxx). Useful
/// only in SmartStart - used when receiving INIF from another network
#define RECEIVE_STATUS_FOREIGN_HOMEID 0x80
///@}

/// \defgroup zwapi_transmit_complete_codes Z-Wave API send_data return codes
///@{
#define TRANSMIT_COMPLETE_OK 0x00  ///< Transmission was successful.
#define TRANSMIT_COMPLETE_NO_ACK \
  0x01  ///< The destination has not acknowledge the frame
#define TRANSMIT_COMPLETE_FAIL    0x02  ///< Transmit error
#define TRANSMIT_ROUTING_NOT_IDLE 0x03  ///< Transmit error
/// Assign route transmit complete but no routes was found
#define TRANSMIT_COMPLETE_NOROUTE  0x04
#define TRANSMIT_COMPLETE_VERIFIED 0x05  ///< Verified delivery
///@}

/// @name ZW_REDISCOVERY_NEEDED callback values
/// Note that they are different from zwapi_request_network_update() callbacks
///@{
#define ZW_ROUTE_LOST_FAILED 0x04  ///< Node Asked wont help us
#define ZW_ROUTE_LOST_ACCEPT 0x05  ///< Accepted to help
///@}

#ifdef ZW_ROUTING_DEMO
/// Max hops in route
#define TRANSMIT_ROUTED_ATTEMPT 0x08
#endif /*ZW_ROUTING_DEMO*/

#define ZW_MAX_CACHED_RETURN_ROUTE_DESTINATIONS 5

#define ZW_RF_TEST_SIGNAL_CARRIER           0x00
#define ZW_RF_TEST_SIGNAL_CARRIER_MODULATED 0x01

/// Max hops in route
#define MAX_REPEATERS 4

/// @name zwapi_tx_report_t Last Used Route array size definitions
///@{
#define LAST_USED_ROUTE_CONF_SIZE 1
#define LAST_USED_ROUTE_SIZE      (MAX_REPEATERS + LAST_USED_ROUTE_CONF_SIZE)
///@}

/// @name zwapi_tx_report_t Last Used Route array index definitions
///@{
#define LAST_USED_ROUTE_REPEATER_0_INDEX 0
#define LAST_USED_ROUTE_REPEATER_1_INDEX 1
#define LAST_USED_ROUTE_REPEATER_2_INDEX 2
#define LAST_USED_ROUTE_REPEATER_3_INDEX 3
///@}

/// @name RSSI feedback constants
/// This is a signed 8-bit value. Note that values from RSSI_RESERVED_START to
/// 124 are reserved. All values below RSSI_RESERVED_START are received power in
/// dBms. Other values are defined below.
///@{
/// RSSI measurement not available
#define RSSI_NOT_AVAILABLE 127
/// Receiver saturated. RSSI too high to measure precisely.
#define RSSI_MAX_POWER_SATURATED 126
/// No signal detected. The RSSI is too low to measure precisely.
#define RSSI_BELOW_SENSITIVITY 125
/// All values above and including RSSI_RESERVED_START are reserved, except
/// those defined above.
#define RSSI_RESERVED_START 11
///@}

/// RSSI value array used in zwapi_tx_report_t. Each value is an RSSI feedback
/// constant defined above.
struct rssi_val {
  int8_t incoming[MAX_REPEATERS + 1];
};

typedef struct _S_ROUTE_LINK_ {
  uint8_t from;
  uint8_t to;
} S_ROUTE_LINK;

/// Transport routing scheme state define definitions
typedef enum _E_ROUTING_SCHEME_ {
  ROUTINGSCHEME_IDLE            = 0,
  ROUTINGSCHEME_DIRECT          = 1,  ///< Direct
  ROUTINGSCHEME_CACHED_ROUTE_SR = 2,  ///< ApplicationStaticRoute
  ROUTINGSCHEME_CACHED_ROUTE    = 3,  ///< ResponseRoute/lastworkingRoute
  ROUTINGSCHEME_CACHED_ROUTE_NLWR
  = 4,                               ///< Next to LastWorkingRoute(controller)
  ROUTINGSCHEME_ROUTE          = 5,  ///< ReturnRoute/controllerAutoRoute
  ROUTINGSCHEME_RESORT_DIRECT  = 6,  ///< directResort
  ROUTINGSCHEME_RESORT_EXPLORE = 7   ///< Explore
} e_routing_scheme_t;

/// List of possible link speeds / protocols
typedef enum last_route_speed {
  UNKNOWN_SPEED                = 0,
  ZWAVE_9_6_KBITS_S            = 1,  ///< Z-Wave 9.6 kbits/s
  ZWAVE_40_KBITS_S             = 2,  ///< Z-Wave 40 kbits/s
  ZWAVE_100_KBITS_S            = 3,  ///< Z-Wave 100 kbits/s
  ZWAVE_LONG_RANGE_100_KBITS_S = 4,  ///< Z-Wave Long Range 100 kbits/s
} last_route_speed_t;

/**
 * @brief Detailed report and data about Z-Wave transmissions
 *
 *
 */
typedef struct zwapi_tx_report {
  ///< Passed 10 ms ticks
  uint16_t transmit_ticks;
  /// Repeaters in route, zero for direct range
  /// rssi_values per hop for direct and routed frames. Contains repeaters + 1
  /// values.
  uint8_t number_of_repeaters;
  // This field is used to indicate ythe RSSI value of the acknowledgement frame
  int8_t ack_rssi;
  struct rssi_val rssi_values;
  // This field is used to indicate the channel number where the ACK received from.
  uint8_t ack_channel_number;
  // This field is used to indicate the channel number that is used to transmit the data.
  uint8_t tx_channel_number;
  e_routing_scheme_t route_scheme_state;
  uint8_t last_route_repeaters[LAST_USED_ROUTE_SIZE];
  // This field indicates if a 250ms beam was used to wake up the destination.
  bool beam_250ms;
  // This field indicates if a 1000ms beam was used to wake up the destination.
  bool beam_1000ms;
  last_route_speed_t last_route_speed;
  uint8_t routing_attempts;
  S_ROUTE_LINK last_failed_link;
  // This field is used to indicate the transmit power used for the transmission
  // The value 127 MUST indicate that the value is not available
  int8_t tx_power;
  // This field is used to indicate the measured noise floor during the outgoing transmission.
  int8_t measured_noise_floor;
  // This field is used to advertise the Tx Power used by the destination in its Ack MPDU frame.
  int8_t destination_ack_mpdu_tx_power;
  // This field is used to advertise the Tx Power used by the destination in its Ack MPDU frame.
  int8_t destination_ack_mpdu_measured_rssi;
  // This field is used to indicate the measured RSSI of the acknowledgement frame received from the destination.
  int8_t destination_ack_mpdu_measured_noise_floor;
} zwapi_tx_report_t;

/**
 * @brief Lock and unlock a response route for a specific node.
 *
 * @param lock bool true for lock entry, if false unlock entry
 * @returns sl_status_t
 *
 * @zgw_name ZW_LockRoute
 */
sl_status_t zwapi_lock_route(bool lock);

/**
 * @brief Transmit data buffer to a single ZW-node or all ZW-nodes (broadcast) in bridge mode.
 *
 * @param source_node_id source NodeID. If set to 0xFF then controller ID is set
 * as source
 * @param destination_node_id destination node ID (0xFF == broadcast)
 * @param data data buffer pointer
 * @param data_length length of data in data buffer
 * @param tx_options transmit option flags:
 *  - @ref TRANSMIT_OPTION_LOW_POWER Transmit at low output power level (1/3 of
 *    normal RF range).
 *  - @ref TRANSMIT_OPTION_ACK Request destination node for an acknowledge that
 *    the frame has been received; callback_function (if not null) will return
 *    result.
 *  - @ref TRANSMIT_OPTION_AUTO_ROUTE Request retransmission via repeater nodes
 *    (at normal output power level).
 * @param callback_function pointer to callback function for when the radio
 * transmission is completed. The callback function has 2 arguments: tx_status
 * (uint8_t) that represents the transmission status.
 * (zwapi_tx_report_t) is the transmit status report.
 * @returns sl_status_t
 *
 * @zgw_name ZW_SendData_Bridge
 */
sl_status_t zwapi_send_data_bridge(
  zwave_node_id_t source_node_id,
  zwave_node_id_t destination_node_id,
  const uint8_t *data,
  uint8_t data_length,
  uint8_t tx_options,
  void (*callback_function)(uint8_t, zwapi_tx_report_t *));

/**
 * @brief Send data to a Z-Wave node or to the broadcast destination.
 *
 * @param destination_node_id destination node ID (0xFF == broadcast)
 * @param data data buffer pointer
 * @param data_length length of data in data buffer
 * @param tx_options transmit option flags:
 *  - @ref TRANSMIT_OPTION_LOW_POWER Transmit at low output power level (1/3 of
 *    normal RF range).
 *  - @ref TRANSMIT_OPTION_ACK Request destination node for an acknowledge that
 *    the frame has been received; callback_function (if not null) will return
 *    result.
 *  - @ref TRANSMIT_OPTION_AUTO_ROUTE Request retransmission via repeater nodes
 *    (at normal output power level).
 * @param callback_function pointer to callback function for when the radio
 * transmission is completed. The callback function has 2 arguments: tx_status
 * (uint8_t) that represents the transmission status. tx_status_report
 * (zwapi_tx_report_t) is the transmit status report.
 *
 * @returns sl_status_t
 *
 * @note: A bridge controller library MUST NOT send to a virtual node that
 * belongs to the bridge itself.
 *
 * @zgw_name ZW_SendData
 */
sl_status_t zwapi_send_data(zwave_node_id_t destination_node_id,
                            const uint8_t *data,
                            uint8_t data_length,
                            uint8_t tx_options,
                            void (*callback_function)(uint8_t,
                                                      zwapi_tx_report_t *));

/**
 * @brief Abort the ongoing transmit started with zwapi_send_data() or zwapi_send_data_bridge()
 *
 * @returns sl_status_t
 *
 * @note If an ongoing transmission is aborted, the callback function from the send
 * call will return with the status TRANSMIT_COMPLETE_NO_ACK.
 *
 * @zgw_name ZW_SendDataAbort
 */
sl_status_t zwapi_abort_send_data(void);

/**
 * @brief Send data to a list of Z-Wave Nodes (multicast frame).
 *
 * @param destination_node_mask node mask where the bits corresponding to the
 * destination node IDs must be set.
 * @param data data buffer pointer
 * @param data_length length of data in data buffer
 * @param tx_options transmit option flags
 * @param callback_function transmit completed call back function
 * @returns sl_status_t
 *
 * @note This function has been adapted so that both bridge and non-bridge
 * Z-Wave API libraries can call it. In case of a bridge library, the function
 * will send from its actual NodeID.
 *
 * @zgw_name SerialAPI_ZW_SendDataMulti
 */
sl_status_t zwapi_send_data_multi(zwave_nodemask_t destination_node_mask,
                                  const uint8_t *data,
                                  uint8_t data_length,
                                  uint8_t tx_options,
                                  void (*callback_function)(uint8_t tx_status));

/**
 * @brief Send data to a list of Z-Wave Nodes (multicast frame) in bridge mode.
 *
 * @param source_node_id source nodeID. If set to 0xFF then controller is set as
 * source
 * @param destination_node_mask node mask where the bits corresponding to the
 * destination node IDs must be set.
 * @param data data buffer pointer
 * @param data_length length of data in data buffer
 * @param tx_options transmit option flags
 * @param callback_function transmit completed call back function
 * @returns sl_status_t
 *
 * @note This function should be used also when the ZPC is the source node
 * because the function zwapi_send_data_multi() is not available on a chip with
 * bridge controller library (which is what the ZPC uses).
 *
 * @zgw_name SerialAPI_ZW_SendDataMulti_Bridge
 */
sl_status_t
  zwapi_send_data_multi_bridge(zwave_node_id_t source_node_id,
                               zwave_nodemask_t destination_node_mask,
                               const uint8_t *data,
                               uint8_t data_length,
                               uint8_t tx_options,
                               void (*callback_function)(uint8_t tx_status));

/**
 * @brief Sends a NOP to a NodeID destination.
 *
 * @returns SL_STATUS_OK if the send_data was initiated.
 * SL_STATUS_FAIL if the send_data was rejected.
 * SL_STATUS_NOT_SUPPORTED if the Z-Wave API command is not supported.
 *
 */
sl_status_t zwapi_send_nop(zwave_node_id_t destination,
                           uint8_t tx_options,
                           void (*callback_function)(uint8_t,
                                                     zwapi_tx_report_t *));

/// @} End of defgroup ZWAPI_TRANSPORT

#ifdef __cplusplus
}
#endif

#endif  // ZWAPI_PROTOCOL_TRANSPORT_H
