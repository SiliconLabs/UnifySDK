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

void zigpc_binding_bind_unbind_callback(void *data);

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
