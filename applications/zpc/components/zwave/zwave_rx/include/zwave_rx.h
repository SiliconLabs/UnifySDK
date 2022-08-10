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
 * @defgroup zwave_rx Z-Wave RX
 * @ingroup zwave_controller
 * @brief Component responsible for initializing the @ref zwave_api, listening to
 * incoming frames, and dispatching them.
 *
 * This component is composed of 2 sub-parts:
 * 1. @ref zwave_rx_fixt : This part is responsible for the @ref zwave_api
 * initialization using the @ref zpc_config for parameters.
 * 2. @ref zwave_rx_process : This component is responsible for continuously polling
 * incoming messages from the Z-Wave module using a @ref contiki process.
 *
 * In case of received Z-Wave frames, they will be passed on to the
 * @ref zwave_transports component handling de-encapsulation of incoming Z-Wave
 * frames (S0, S2, Transport Service, Multi Channel, CRC16, Multi Command)
 *
 * For some other events, such as received NIF, SmartStart inclusion or node deleted
 * from the network, the @ref zwave_controller will be notified.
 *
 * Receiving a frame is illustrated in the picture below:
 * \image html zwave_rx_receiving_a_frame.png "Receiving a Z-Wave Command"
 *
 * @{
 */

#ifndef ZWAVE_RX_H
#define ZWAVE_RX_H

#include "zwave_controller_connection_info.h"
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Describing receive parameters of a frame.
 *
 */
typedef struct {
  /// Received frame status flags
  ///
  /// - RECEIVE_STATUS_ROUTED_BUSY A response route is locked by the application
  /// - RECEIVE_STATUS_LOW_POWER Received at low output power level
  /// - RECEIVE_STATUS_TYPE_SINGLE Received a single cast frame
  /// - RECEIVE_STATUS_TYPE_BROAD Received a broadcast frame
  /// - RECEIVE_STATUS_TYPE_MULTI Received a multicast frame
  /// - RECEIVE_STATUS_TYPE_EXPLORE Received an explore frame
  /// - RECEIVE_STATUS_FOREIGN_FRAME The received frame is not addressed to this
  ///   node (Only valid in promiscuous mode)
  /// - RECEIVE_STATUS_FOREIGN_HOMEID The received frame is received from a
  ///   foreign HomeID. Only Controllers in SmartStart AddNode mode can receive this
  uint8_t status_flags;

  /// RSSI measurement of the received frame This is a signed 8-bit value.
  ///
  /// Values from RSSI_RESERVED_START to 124 are reserved.
  /// - All values below RSSI_RESERVED_START are received power in dBm.
  /// - RSSI_NOT_AVAILABLE - RSSI measurement not available
  /// - RSSI_MAX_POWER_SATURATED - Receiver saturated. RSSI too high to measure precisely
  /// - RSSI_BELOW_SENSITIVITY - No signal detected. The RSSI is too low to measure precisely.
  int8_t rssi;

  /// Node list for z-wave multicast frames.
  ///
  zwave_node_list_t nodes_in_multicast;
} zwave_rx_receive_options_t;

/**
 * @brief Waits for the Z-Wave API to have restarted/be ready
 * after a soft reset.
 *
 * NOTE: This function will be blocking for up to a couple of seconds.
 */
void zwave_rx_wait_for_zwave_api_to_be_ready();

#ifdef __cplusplus
}
#endif
/** @} end zwave_rx */

#endif  //ZWAVE_RX_H
