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

#ifndef ZWAVE_COMMAND_HANDLER_H
#define ZWAVE_COMMAND_HANDLER_H

/**
 * @defgroup zwave_command_handler Z-Wave Application Command Class Handler
 * @ingroup components
 * @brief Application Command Class handler framework,
 *  dispatching incoming Z-Wave Commands to the corresponding handler.
 *
 * This component takes care of keeping track of the list of supported
 * and controlled Command Classes.
 *
 * When a frame is received, it is in charge of verifying the security at
 * which the frame has been received and forward the Z-Wave Command accordingly.
 *
 * @{
 */

// Includes from other components
#include "zwave_controller_connection_info.h"
#include "zwave_controller.h"
#include "zwave_rx.h"
#include "sl_status.h"

// Generic includes
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   * @brief This is the function which will be executed when the frame of
   * a given Command Class is received.
   *
   * The handler MUST return a \ref sl_status_t status code.
   *
   * @param connection_info Info about the connection properties of this frame.
   * @param frame_data The data payload of this frame.
   * @param frame_length The length of this frame.
   *
   * @returns SL_STATUS_OK    The command was handled by the command handler.
   *                          Supervision will return SUCCESS in this case
   * @returns SL_STATUS_FAIL  The command handler was unable to parse the command
   *                          or was busy carring another operation.
   *                          Supervision Command Class returns FAIL.
   * @returns SL_STATUS_IN_PROGRESS   The command handler is processing the command
   *                                  Supervision Command Class returns WORKING.
   * @returns SL_STATUS_NOT_SUPPORTED The command handler does not support this
   *                                  Command or Command Class.
   *                                  Supervision Command Class returns NO_SUPPORT.
   * @returns Any other status: Supervision Command Class returns FAIL.
   */
typedef sl_status_t (*zwave_command_class_handler_t)(
  const zwave_controller_connection_info_t *connection,
  const uint8_t *frame_data,
  uint16_t frame_length);

/**
   * @brief Z-Wave Command Class handler object
   *
   * This structure is a package allowing the Z-Wave Command Class dispatch to accept or
   * reject Command frames from the Z-Wave radio and dispatch them to the right handler, if
   * they are supported.
   */
typedef struct {
  /// Handler for supported frames.
  ///
  zwave_command_class_handler_t support_handler;

  /// Handler for controlled frames.
  ///
  zwave_command_class_handler_t control_handler;

  /// The minimal security level which this command is supported on.
  /// This is ignored for the control_handler.
  ///
  zwave_controller_encapsulation_scheme_t minimal_scheme;

  /// command class that this handler implements
  ///
  zwave_command_class_t command_class;

  /// Name of the Command Class (not including Command Class)
  ///
  const char *command_class_name;

  /// Comments for the Command Class implemenation, that is printed to the log
  ///
  const char *comments;

  /// version of the implemented command class
  ///
  uint8_t version;

  /// Use manual-security filtering for incoming frames
  /// If set to true, the command class dispatch handler will send frames to the
  /// handler without validating their security level.
  /// If set to false, the command class handler can assume that the frame has
  /// been received at an approved security level.
  bool manual_security_validation;
} zwave_command_handler_t;

/**
 * @brief Initialize the command handlers.
 * @returns SL_STATUS_OK, it will always be considered as successful.
 */
sl_status_t zwave_command_handler_init(void);

/**
 * @brief Teardown of the Z-Wave command handler.
 * @returns 0 in case of success.
 */
int zwave_command_handler_teardown(void);

/**
 * @brief Register a command class handler.
 *
 * This API replaces the legacy REGISTER_HANDLER() macro. Command
 * handlers should perform their own initialization and register
 * themselves to the command handler framework.
 *
 * There is no sequence/order requirement, meaning that this function
 * can be invoked before the Command Class handler framework is initialized.
 *
 * @param new_command_class_handler A struct that contains the
 *                                  zwave_command_handler_t info
 *                                  that will be used for registering a
 *                                  Command Class handler.
 *
 * Command class handlers can be fully disabled by setting
 * both support and control to false.
 * @returns SL_STATUS_OK    If the Command Class handler was successfully
 *                          registered
 * @returns SL_STATUS_FAIL  Otherwise
 */
sl_status_t zwave_command_handler_register_handler(
  zwave_command_handler_t new_command_class_handler);

/**
 * @brief Get the version of Command Class that the handler handles.
 *
 * @param command_class Command class to query with.
 * @returns The version number of the indicated Command Class.
 *          If the Command Class is neither supported not controlled, it will be 0.
 *          0 if no Command Class handler is registered for the Command Class identifier.
 */
uint8_t zwave_command_handler_get_version(zwave_command_class_t command_class);

// It is moved here to insert supervision decapsulated commands back to
// the command handler. This may be updated with TX/RX validation scheme
// component work.
/**
 * @brief Dispatches a frame to its respective Command Class handler
 *
 * @param connection_info The connection information for the received Z-Wave
 *                        Frame
 * @param frame_data      The payload of the Z-Wave Frame
 * @param frame_length    The length of the payload (in bytes) contained
 *                        in the frame_data pointer.
 * @returns The handler return code
 */
sl_status_t zwave_command_handler_dispatch(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length);

/**
 * @brief Check if we control a given command class
 *
 * @param command_class
 * @return true if we control this command classs
 */
bool zwave_command_handler_controls(zwave_command_class_t command_class);

/**
 * @brief Print Command Class Version info
 *
 * @param fd File descriptor to print to, if fd < 0 it will use sl_log_info
 */
void zwave_command_handler_print_info(int fd);

#ifdef __cplusplus
}
#endif

/** @} end zwave_command_handler */

#endif /* ZWAVE_COMMAND_HANDLER_H */
