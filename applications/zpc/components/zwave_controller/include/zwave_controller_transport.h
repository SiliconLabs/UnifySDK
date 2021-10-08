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
 * @defgroup zwave_controller_transport Z-Wave Controller Transport
 * @ingroup zwave_controller
 *
 * @brief Z-Wave controller transport framework.
 *
 * The purpose of the Z-wave controller transport framework, is to have a simple
 * API witch handles integration of all Z-Wave transport command classes. Each
 * encapsulation command class will register its command handlers and send
 * data function to the controller. The controller module will make sure that
 * the right handler will be executed when appropriate.
 *
 * Each transport can be single session when sending.
 *
 * All transport must use the @ref zwave_tx_send_data function when sending
 * frames. It is important the the transport handlers use the
 * zwave_tx_parent_session_id parameter in zwave_tx_options_t when sending
 * frames.
 *
 * When a frame is de-encapsulated it must be forwarded using the @ref
 * zwave_controller_on_frame_received function.
 *
 * \image html zwave_controller_transport.png "Interactions of the Z-Wave transport framework"
 * \image html zwave_transport_plugin.png "Transport plugin interactions"
 *
 * @{
 */

#ifndef ZWAVE_CONTROLLER_TRANSPORT_H
#define ZWAVE_CONTROLLER_TRANSPORT_H

#include "zwave_controller.h"
#include "zwave_tx.h"

#include "zwave_command_class_version_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Hold information about a transport plugin.
 *
 */
typedef struct zwave_controller_transport {
  /**
   * @brief Priority of this plugin, transports with lowest number priority is
   * executed last when encoding.
   * Decoding order is made based on the Command Class identifier
   * (first frame byte).
   *
   * Recommended priorities:
   * - 1 Transport Service: TBD: Connection info should have fields with
   *                        "segmentation settings" or something like this
   *                        Else the transport can't really know if this is
   *                        supported by the destination
   * - 2 Security 2
   * - 3 Security 0
   * - 4 CRC 16
   * - 5 Multi Channel
   *
   */
  uint32_t priority;
  /**
   * @brief Which Z-Wave command class this transport is serving
   *
   */
  zwave_command_class_t command_class;
  /**
   * @brief The command class version of the command class that this transport
   * is implementing.
   *
   */
  zwave_cc_version_t version;

  /**
   * @brief Send function to use when encapsulating with this transport.
   *
   * @note: It is important for sender function not to pass the parent_session
   * to the zwave_tx send_data function.
   *
   * Functions registered as send_data handlers MUST return status
   * codes according to the following:
   *  - SL_STATUS_OK              indicates that the frame was accepted and
   *                              encapsulated
   *  - SL_STATUS_NOT_SUPPORTED   indicates that transport has nothing to
   *                              encapsulate for this frame.
   *  - SL_STATUS_WOULD_OVERFLOW  indicates that the frame is supported but cannot
   *                              be handled, so it should not be attempted again
   *  - Any other error code      (SL_STATUS_BUSY, SL_STATUS_FAIL, etc.) indicating
   *                              that the frame should be encapsulated,
   *                              but could not due to an error.
   */
  sl_status_t (*send_data)(
    const zwave_controller_connection_info_t *connection,
    uint16_t data_length,
    const uint8_t *data,
    const zwave_tx_options_t *tx_options,
    const on_zwave_tx_send_data_complete_t on_send_complete,
    void *user,
    zwave_tx_session_id_t parent_session);

  /**
   * @brief Command handler to of the transport
   *
   * Functions registered as on_frame_received transport MUST return status
   * codes according to the following:
   * - SL_STATUS_NOT_FOUND    Indicates that the frame has no more
   *                          encapsulation and must be passed manually to the
   *                          upper layers by the Z-Wave Controller.
   * - Any other code         (SL_STATUS_OK, SL_STATUS_FAIL, etc.) indicating
   *                          that the frame was "consumed" by the
   *                          on_frame_received handler and that the handler
   *                          has passed the frame to the Z-Wave Controller
   *                          again if decapsulation was applied (i.e. called
   *                          zwave_controller_on_frame_received())
   */
  sl_status_t (*on_frame_received)(
    const zwave_controller_connection_info_t *connection_info,
    const zwave_rx_receive_options_t *rx_options,
    const uint8_t *frame_data,
    uint16_t frame_length);
  /* data */
} zwave_controller_transport_t;

/**
 * @brief Must be called to register a transport.
 *
 * There can only be one transport pr command class and only one pr. priority.

 * @param transport
 * @return sl_status_t
 */
sl_status_t zwave_controller_transport_register(
  const zwave_controller_transport_t *transport);

/**
 * @brief check if a command class is registered as an encapsulation cc in
 *        Z-Wave controller transport framework. If it does, the module will
 *        retrun the version of the encapsulation command class.
 *
 * @param  command_class Z-Wave Command Class
 *
 * @return zero if the command class is not registered as an encapsulation cc
 *              in the Z-Wave transport plugin framework.
 */
zwave_cc_version_t zwave_controller_transport_is_encapsulation_cc(
  zwave_command_class_t command_class);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_CONTROLLER_TRANSPORT_H
/** @} end zwave_controller_transport */
