/******************************************************************************
* # License
* <b>Copyright 2021  Silicon Laboratories Inc. www.silabs.com</b>
******************************************************************************
* The licensor of this software is Silicon Laboratories Inc. Your use of this
* software is governed by the terms of Silicon Labs Master Software License
* Agreement (MSLA) available at
* www.silabs.com/about-us/legal/master-software-license-agreement. This
* software is distributed to you in Source Code format and is governed by the
* sections of the MSLA applicable to Source Code.
*
*****************************************************************************/

// Includes from this component
#include "zwave_multi_channel_transport.h"

// Generic includes
#include <stdbool.h>
#include <string.h>

// Includes from other components
#include "ZW_classcmd.h"
#include "zwave_controller_command_class_indices.h"
#include "zwave_controller_connection_info.h"
#include "zwave_controller_transport.h"
#include "zwave_controller_internal.h"
#include "zwave_rx.h"
#include "zwave_tx.h"

// Send data TX callback arrays
// We support multiple send data session at the same time. We save callback settings
// here in the static array.
typedef struct send_data_callback_setting {
  on_zwave_tx_send_data_complete_t on_send_data_complete;
  void *user;
} send_data_callback_setting_t;

static send_data_callback_setting_t
  callback_list[MAXIMUM_MULTI_CHANNEL_SEND_SESSIONS];

///////////////////////////////////////////////////////////////////////////////
// Private helper functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Search our list of callback_settings and returns the address of the
 *        first available slot
 *
 * @param on_send_data_complete function to save for the callback in the
 *                              callback setting lot
 * @param user                  User pointer to save for the callback in the
 *                              callback setting lot
 *
 * @returns send_data_callback_setting_t pointer allocated for a new session.
 * @returns NULL if all our sessions are in use
 */
static send_data_callback_setting_t *get_first_available_send_data_setting_slot(
  on_zwave_tx_send_data_complete_t on_send_data_complete, void *user)
{
  for (uint8_t i = 0; i < MAXIMUM_MULTI_CHANNEL_SEND_SESSIONS; i++) {
    if (callback_list[i].on_send_data_complete == NULL) {
      callback_list[i].on_send_data_complete = on_send_data_complete;
      callback_list[i].user                  = user;
      return &(callback_list[i]);
    }
  }
  return NULL;
}

/**
 * @brief Initializes our array of callback settings by setting
 *        everything to NULL
 */
static void initialize_send_data_settings()
{
  for (uint8_t i = 0; i < MAXIMUM_MULTI_CHANNEL_SEND_SESSIONS; i++) {
    callback_list[i].on_send_data_complete = NULL;
    callback_list[i].user                  = NULL;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Z-Wave Controller transport functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @defgroup multi_channel_transport Multi Channel Transport
 * @ingroup multi_channel_command_class
 * @brief Multi Channel encapsulation and decapsulation module
 *
 * This module allows to send and receive Multi Channel encapsulated
 * frames.
 *
 * @{
 */

/**
 * @brief Callback function registered to Z-Wave TX \ref zwave_tx_send_data
 *
 * It helps tracking when a frame was fully transmitted and will invoke the
 * callback of the component that called the \ref
 * zwave_command_class_multi_channel_send_data function
 *
 * @param status  Indicates how the transmission operation was completed.
 *                Refer for \ref zwapi_transmit_complete_codes for details.
 * @param tx_info zwapi_tx_report_t reported by the \ref ZWAPI. It
 *                contains transmission details, refer to \ref zwapi_tx_report_t.
 * @param user    User pointer provided in \ref zwave_command_class_multi_channel_send_data()
 */
static void on_multi_channel_send_complete(uint8_t status,
                                           const zwapi_tx_report_t *tx_info,
                                           void *user)
{
  // Call the registered callback directly and tell them we are happy with the
  // transmission of our Multi Channel encapsulated frame. No retry or
  // additional frames needed
  send_data_callback_setting_t *callback_setting
    = (send_data_callback_setting_t *)user;

  // We do not want to trust the pointer and invoke whatever function
  // so we search our array to check if we can find the same address
  // before we invoke anything
  for (uint8_t i = 0; i < MAXIMUM_MULTI_CHANNEL_SEND_SESSIONS; i++) {
    if (&(callback_list[i]) == callback_setting) {
      if (callback_setting->on_send_data_complete != NULL) {
        callback_setting->on_send_data_complete(status,
                                                tx_info,
                                                callback_setting->user);
        callback_setting->on_send_data_complete = NULL;
        callback_setting->user                  = NULL;
      }
      return;
    }
  }
}

/**
 * @brief Encapsulates with Multi Channel and send a frame to the Z-Wave TX queue.
 *
 * The provided payload will be Multi Channel encapsulated using the endpoint
 * data provided in
 * connection->remote.endpoint_id and connection->local.endpoint_id.
 *
 * Bit addressing can be used by setting the endpoint value directly (1 bit
 * bit addressing and 7 bits endpoint identifier)
 *
 * @param connection       Connection object describing the source and
 *                         destination. If either the source or destination
 *                         endpoints ID are not null, the frame will
 *                         be Multi Channel encapsulated.
 * @param data_length      Length of the frame to send
 * @param data             Points to the payload to send
 * @param tx_options       Transmit options to use.
 * @param on_multi_channel_complete Callback function that will be called when
 *                                  the send operation has completed
 * @param user             User pointer passed in argument of the on_send_complete
 *                         callback function
 * @param parent_session_id Value of the frame in the TX Queue that is the parent
 *                          of this frame. Frames MUST have a valid parent
 *
 * @returns
 * - SL_STATUS_OK The transmission request has been accepted and callback will be
 *                    triggered when the operation is completed.
 * - SL_STATUS_NOT_SUPPORTED   If no endpoint encapsulation is to be applied
 * - SL_STATUS_BUSY            If the transmission cannot be done at the moment.
 */
static sl_status_t zwave_command_class_multi_channel_send_data(
  const zwave_controller_connection_info_t *connection,
  uint16_t data_length,
  const uint8_t *data,
  const zwave_tx_options_t *tx_options,
  const on_zwave_tx_send_data_complete_t on_multi_channel_complete,
  void *user,
  zwave_tx_session_id_t parent_session_id)
{
  // Exception time! If we are sending a Multi Channel CC command to an
  // endpoint > 0, we do not encapsulate the destination endpoint because this
  // command it has to be sent to the root device for any of its endpoints
  zwave_endpoint_id_t destination_endpoint = connection->remote.endpoint_id;
  if ((data_length >= 2)
      && (data[COMMAND_CLASS_INDEX] == COMMAND_CLASS_MULTI_CHANNEL_V4)) {
    destination_endpoint = 0;
  }
  //Same thing for Version Command Class Get, they are sent to the Root Device
  if ((data_length >= 2)
      && (data[COMMAND_CLASS_INDEX] == COMMAND_CLASS_VERSION_V3)
      && (data[COMMAND_INDEX] == VERSION_COMMAND_CLASS_GET_V3)) {
    destination_endpoint = 0;
  }

  // Does the frame need Multi Channel encapsulation ?
  if ((connection->local.endpoint_id == 0) && (destination_endpoint == 0)) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  // Is the frame too big for us?
  if (data_length > MULTI_CHANNEL_ENCAPSULATED_COMMAND_MAXIMUM_SIZE) {
    return SL_STATUS_WOULD_OVERFLOW;
  }

  send_data_callback_setting_t *current_setting
    = get_first_available_send_data_setting_slot(on_multi_channel_complete,
                                                 user);

  if (current_setting == NULL) {
    return SL_STATUS_BUSY;
  }

  zwave_tx_options_t multi_channel_tx_options       = *tx_options;
  multi_channel_tx_options.parent_session_id        = parent_session_id;
  multi_channel_tx_options.valid_parent_session_id  = true;
  multi_channel_tx_options.use_parent_frame_options = true;

  // Set the endpoint data to 0 now that we have encapsulated it.
  zwave_controller_connection_info_t new_connection = *connection;
  new_connection.remote.endpoint_id                 = 0;
  new_connection.local.endpoint_id                  = 0;

  zwave_multi_channel_encapsulation_frame_t frame = {0};
  uint16_t frame_length                           = 0;

  frame.command_class        = COMMAND_CLASS_MULTI_CHANNEL_V4;
  frame.command              = MULTI_CHANNEL_CMD_ENCAP_V4;
  frame.source_endpoint      = connection->local.endpoint_id & ENDPOINT_MASK;
  frame.destination_endpoint = destination_endpoint;
  memcpy(frame.encapsulated_command, data, data_length);
  frame_length = MULTI_CHANNEL_ENCAPSULATION_OVERHEAD + data_length;

  return zwave_tx_send_data(&new_connection,
                            frame_length,
                            (uint8_t *)&frame,
                            &multi_channel_tx_options,
                            &on_multi_channel_send_complete,
                            current_setting,
                            NULL);
}

/**
 * @brief Decapsulate Multi Channel encapsulation and inject the frame back to
 *        the Z-Wave Controller
 *
 * The provided payload will be Multi Channel decapsulated and the endpoint
 * data will be copied in
 * connection->remote.endpoint_id and connection->local.endpoint_id and passed
 * to the \ref zwave_controller_on_frame_received() function.
 *
 * Bit addressing can be used by setting the endpoint value directly (1 bit
 * bit addressing and 7 bits endpoint identifier)
 *
 * @param connection_info  Connection object describing the source and
 *                         destination.
 * @param rx_options       Connection object describing the source and
 *                         destination.
 * @param frame_data       Length of the frame to send
 * @param frame_length     Points to the payload to send
 *
 * @returns
 * - SL_STATUS_NOT_SUPPORTED  if the frame data is not multi channel encapsulated
 *                            properly to generate a decapsulated frame
 * - SL_STATUS_NOT_FOUND      if the frame is not encapsulated but should not be
 *                            discarded.
 * - SL_STATUS_WOULD_OVERFLOW If the decapsulated frame is too large to fit in
 *                            our local buffer
 * - SL_STATUS_OK             If the frame was decapsulated and should be discarded
 *                            because its decapsulated version has been passed
 *                            to the Z-Wave Controller.
 */
static sl_status_t zwave_command_class_multi_channel_decapsulate(
  const zwave_controller_connection_info_t *connection_info,
  const zwave_rx_receive_options_t *rx_options,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length < 5) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (frame_data[COMMAND_CLASS_INDEX] != COMMAND_CLASS_MULTI_CHANNEL_V4
      || frame_data[COMMAND_INDEX] != MULTI_CHANNEL_CMD_ENCAP_V4) {
    // SL_STATUS_NOT_FOUND will pass the frame to the upper layer,
    // i.e. it will go to the Multi Channel CC handler
    return SL_STATUS_NOT_FOUND;
  }

  zwave_controller_connection_info_t info = *connection_info;
  zwave_rx_receive_options_t options      = *rx_options;

  uint8_t decapsulated_frame[ZWAVE_MAX_FRAME_SIZE] = {0};
  uint16_t decapsulated_frame_length               = 0;

  info.remote.endpoint_id = frame_data[2] & ENDPOINT_MASK;
  info.local.endpoint_id  = frame_data[3];

  if ((info.local.endpoint_id & BIT_ADDRESS) == BIT_ADDRESS) {
    info.local.is_multicast = true;
  }

  if (frame_length - MULTI_CHANNEL_ENCAPSULATION_OVERHEAD
      > sizeof(decapsulated_frame)) {
    return SL_STATUS_WOULD_OVERFLOW;
  }
  decapsulated_frame_length
    = frame_length - MULTI_CHANNEL_ENCAPSULATION_OVERHEAD;
  memcpy(decapsulated_frame,
         &frame_data[MULTI_CHANNEL_ENCAPSULATION_OVERHEAD],
         decapsulated_frame_length);

  zwave_controller_on_frame_received(&info,
                                     &options,
                                     decapsulated_frame,
                                     decapsulated_frame_length);

  return SL_STATUS_OK;
}
/** @} end multi_channel_transport */

///////////////////////////////////////////////////////////////////////////////
// Shared functions within the component
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_multi_channel_transport_init()
{
  initialize_send_data_settings();

  // Register our transport to the Z-Wave Controller Transport
  zwave_controller_transport_t transport = {0};
  transport.priority                     = 5;
  transport.command_class                = COMMAND_CLASS_MULTI_CHANNEL_V4;
  transport.version                      = MULTI_CHANNEL_VERSION;
  transport.on_frame_received = &zwave_command_class_multi_channel_decapsulate;
  transport.send_data         = &zwave_command_class_multi_channel_send_data;

  return zwave_controller_transport_register(&transport);
}
