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
 * @defgroup zwave_s2_keystore Security 2 Keystore
 * @ingroup zwave_transports
 * @brief Z-Wave Keystore
 *
 * The Z-Wave keystore is storing, accesing and gerating the Z-Wave network
 * keys as well as the ECDH keys which are used in Security 2. All keys are
 * stored in the NVM of the Z-Wave module.
 *
 * @{
 */

#ifndef ZWAVE_S2_KEYSTORE_H
#define ZWAVE_S2_KEYSTORE_H

#include "zwave_controller_types.h"
#include "sl_log.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ECDH key mode types
 */
typedef enum {
  /// This is a static key used for being included when at least one
  /// Authenticated Security Class is granted. It matches the ZPC's DSK
  /// exchanged out of band. This key is fetched from the NVM of the module.
  ZWAVE_S2_KEYSTORE_STATIC_ECDH_KEY,
  /// This is a dynamic key used for
  /// 1. Including new nodes (add mode)
  /// 2. Being included when no Authenticated Security Class is granted. The key
  ///    must be regenerated after each S2 bootstrapping attempt
  ZWAVE_S2_KEYSTORE_DYNAMIC_ECDH_KEY,
} zwave_s2_keystore_ecdh_key_mode_t;

/**
 * @brief Return one of the DSKs of the controller.
 *
 * This function can be used fetch the DSK of the controller. There are two use
 * cases of this, as follows:
 *  1. When including the controller into an other z-wave network, in this case
 *     the  ZWAVE_S2_KEYSTORE_STATIC_ECDH_KEY should be used.
 *  2. When including a node using CSA inclusion, In this case the
 *     ZWAVE_S2_KEYSTORE_DYNAMIC_ECDH_KEY should be used.
 *
 * @param mode Which dsk to get.
 * @param dsk  pointer to a zwave_dsk_t to copy the dsk to.
 */
void zwave_s2_keystore_get_dsk(zwave_s2_keystore_ecdh_key_mode_t mode,
                               zwave_dsk_t dsk);

/**
 * @brief Get the set of the currently assigned keys.
 *
 * @return uint8_t Bit field of keys
 */
uint8_t zwave_s2_keystore_get_assigned_keys();

/**
 * @brief Resets the list of assigned keys (sets them to 0)
 * Use this function before receiving keys when entering in a network.
 */
void zwave_s2_keystore_reset_assigned_keys();

/**
 * Create new random network keys, which marks all keys as granted.
 */
void zwave_s2_create_new_network_keys();

/**
 * Logs the Security Keys using sl_log.
 * @param log_level   The Log leve to use to log the keys.
 */
void zwave_s2_log_security_keys(sl_log_level_t log_level);

/**
 * Save the Z-Wave Security Keys in a file in Zniffer friendly format.
 * @param filename Name/path of the file where keys should be saved 
 */
void zwave_s2_save_security_keys(const char *filename);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_S2_KEYSTORE_H
/** @} end zwave_s2_keystore */
