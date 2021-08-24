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
 * @file zwave_s2_keystore.h
 * @addtogroup zwave_s2_keystore
 * @ingroup components
 * @brief Internal functions for the Keystore
 *
 * @{
 */

#ifndef ZWAVE_S2_KEYSTORE_INT_H
#define ZWAVE_S2_KEYSTORE_INT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "zwave_s2_keystore.h"
#include "zwapi_protocol_mem.h"

// NVM magic value, this is the same a ZGW
#define NVM_MAGIC 0xa11d1900
typedef struct nvm_config {
  uint32_t magic;
  uint8_t filler1[41];
  uint8_t security_netkey[16];
  uint8_t filler2[23];
  /*S2 Network keys */
  uint8_t assigned_keys;  //Bitmask of keys which are assigned
  uint8_t security2_key[3][16];
  uint8_t ecdh_priv_key[32];
  uint8_t security2_lr_key[2][16];
} __attribute__((__packed__)) nvm_config_t;

#define nvm_config_get(par_name, dst)                            \
  zwapi_memory_get_buffer(offsetof(struct nvm_config, par_name), \
                          (uint8_t*)dst,                         \
                          sizeof(((struct nvm_config *)0)->par_name))
#define nvm_config_set(par_name, src)                                 \
  zwapi_memory_put_buffer(offsetof(struct nvm_config, par_name),      \
                          (uint8_t*)src,                               \
                          sizeof(((struct nvm_config *)0)->par_name), \
                          NULL)

/**
 * @brief Initialize the keystore
 * 
 * This also validates the nvm contents. If the validation fails new keys are g
 * generated.
 */
void zwave_s2_keystore_init();

/**
 * Create a new Elliptic curve key for add node mode
 * and for unauthenticated learnmode.
 */
void zwave_s2_create_new_dynamic_ecdh_key();

/**
 * @brief Set ecdh key mode for use in zwave_s2_keystore
 *
 * @param mode ecdh key mode to use
 */
void zwave_s2_keystore_set_ecdh_key_mode(
  zwave_s2_keystore_ecdh_key_mode_t mode);


#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_S2_KEYSTORE_INT_H
/** @} end zwave_s2_keystore */
