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
 * @file zigpc_protctrl_nwmgmt_write.h
 * @defgroup zigpc_protctrl_nwmgmt_write_callback ZigPC NetworkManagement/Write
 * Callback Handler
 * @ingroup zigpc_protctrl_nwmgmt_write
 * @brief The ZigPC Protocol Controller NetworkManagement/Write callback
 * handler is responsible for extracting any known state changes and its
 * associated state paramteres from incoming NetworkManagement/Write messages
 * (such as: add a node, remove a node, etc.)
 *
 * @{
 */

#ifndef ZIGPC_PROTCTRL_NWMGMT_WRITE_CALLBACK_H
#define ZIGPC_PROTCTRL_NWMGMT_WRITE_CALLBACK_H

#include "sl_status.h"

#include "zigpc_common_zigbee.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Container for the state parameters extracted from any incoming
 * NetworkManagement Write messages according to Unify spec section 3.5.1.
 * Since each state parameter is indicated as a key-value pair in JSON format,
 * the following container stores the key and value as character arrays along
 * with their appropriate size.
 */
typedef struct {
  const char *key;
  size_t key_size;
  const char *value;
  size_t value_size;
} zigpc_protctrl_state_parameter_t;

/**
 * @brief Callback to be implemented to receive the latest state and associated
 * state parameters as character strings. It is the responsibilty of this
 * callback to validate the passed in state and state parameters and perform
 * any additional actions if necessary.
 *
 * NOTE: This callback will be invoked by zigpc_protctrl_nwmgmt_write_update
 * after successful parsing and state parameter extraction.
 *
 * @param state                 Character array representing the extracted state.
 * @param state_parameter_list  List of state parameter key value pairs.
 * @param state_parameter_count Count of state parameters in list.
 * @return sl_status_t          SL_STATUS_OK if the callback processes the
 * passed information successfully. If any other status is returned, a warning
 * message will be logged.
 */
sl_status_t zigpc_protctrl_on_write_update_callback(
  const char *state,
  const zigpc_protctrl_state_parameter_t *state_parameter_list,
  uint8_t state_parameter_count);

#ifdef __cplusplus
}
#endif

#endif /* ZIGPC_PROTCTRL_NWMGMT_WRITE_CALLBACK_H */

/** @} end zigpc_protctrl_nwmgmt_write_callback */
