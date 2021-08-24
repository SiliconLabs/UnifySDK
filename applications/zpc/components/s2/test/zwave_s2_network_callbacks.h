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
 * This is only used for making cmock test functions
 * @{
 */
 
#ifndef ZWAVE_S2_NETWORK_CALLBACKS_H
#define ZWAVE_S2_NETWORK_CALLBACKS_H

#include "zwave_network_management.h"

#ifdef __cplusplus
extern "C" {
#endif

void on_inclusion_started();
void on_inclusion_complete(zwave_keyset_t granted_keys,
                              zwave_kex_fail_type_t kex_fail_code);
void on_keys_request(zwave_keyset_t requested_keys, bool csa);

void on_dsk_challenge(zwave_keyset_t granted_keys,
                          int num_blank_bytes,
                          zwave_dsk_t dsk);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_S2_NETWORK_CALLBACKS_H
/** @} end zwave_s2_network_callbacks */
