/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#ifndef ZIGPC_BINDING_RESPONSE_H 
#define ZIGPC_BINDING_RESPONSE_H 

#include "zigpc_common_zigbee.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief zigpc_binding_bind_unbind_callback
 * A callback used on the reception of a binding or unbinding command. Contains
 * the raw information from the gateway
 *
 * @param data - a pointer to the event data containing the binding information
 * Note, this function should free any memory assigned to data
 */ 
void zigpc_binding_bind_unbind_callback(void *data);

/**
 * @brief zigpc_binding_handle_bind_unbind_reponse
 * Handle a response from a binding/binding command. Handles the parsed response
 * after being processed
 *
 * @param binding           - the binding information
 * @param is_bind_response  - a boolean representing if a binding (true) or
 *                            unbinding (false) response
 * @param zdo_status        - the ZDO status contained in the response
 *
 * @return SL_STATUS_OK if able to properly handle the response, SL_STATUS_FAIL
 * if something went wrong in parsing the binding or if the binding request
 * itself failed in some way
 */ 
sl_status_t zigpc_binding_handle_bind_unbind_reponse(
                const zigbee_binding_t binding,
                bool is_bind_response,
                uint8_t zdo_status);

sl_status_t zigpc_binding_remove_binding(const zigbee_binding_t binding);
sl_status_t zigpc_binding_add_binding(const zigbee_binding_t binding);

#ifdef __cplusplus
}
#endif

#endif //ZIGPC_BINDING_RESPONSE_H
