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

#include "zwave_s2_keystore.h"
#include "zwave_s2_keystore_int.h"

/// Must be included before S2.h (because of definition of offsetof)
#include "stddef.h"

/// libs2 includes
#include "S2.h"
#include "curve25519.h"
#include "s2_keystore.h"

/// zwave_api includes
#include "zwapi_init.h"
#include "zwapi_protocol_basis.h"
#include "zwapi_protocol_mem.h"

// S0 includes (S0 network key set)
#include "zwave_s0_transport.h"

/// Other
#include "sl_log.h"
#include "zpc_endian.h"
#define LOG_TAG "zwave_s2_keystore"

#define S2_NUM_KEY_CLASSES (s2_get_key_count()) /* Includes the S0 key */
#define NETWORK_KEY_SIZE   16  //< Size of a Z-Wave network key.

uint8_t dynamic_ecdh_private_key[32];
static zwave_s2_keystore_ecdh_key_mode_t ecdh_key_mode;

bool keystore_network_key_clear(uint8_t keyclass)
{
  uint8_t random_bytes[64];
  uint8_t assigned_keys = 0xff;

  AES_CTR_DRBG_Generate(&s2_ctr_drbg, random_bytes);

  if (keyclass == KEY_CLASS_ALL) {
    keystore_network_key_clear(KEY_CLASS_S0);
    keystore_network_key_clear(KEY_CLASS_S2_UNAUTHENTICATED);
    keystore_network_key_clear(KEY_CLASS_S2_AUTHENTICATED);
    keystore_network_key_clear(KEY_CLASS_S2_ACCESS);
    keystore_network_key_clear(KEY_CLASS_S2_AUTHENTICATED_LR);
    keystore_network_key_clear(KEY_CLASS_S2_ACCESS_LR);
    return true;
  }

  if (keystore_network_key_write(keyclass, random_bytes)) {
    nvm_config_get(assigned_keys, &assigned_keys);
    assigned_keys &= ~keyclass;
    nvm_config_set(assigned_keys, &assigned_keys);
    memset(random_bytes, 0, sizeof(random_bytes));
    return true;
  }
  return false;
}

void keystore_private_key_read(uint8_t *buf)
{
  uint8_t nvr_version;

  if (ecdh_key_mode == ZWAVE_S2_KEYSTORE_DYNAMIC_ECDH_KEY) {
    memcpy(buf, dynamic_ecdh_private_key, 32);
  } else {
    uint8_t my_chip_type = 0;
    uint8_t version      = 0;
    zwapi_get_chip_type_version(&my_chip_type, &version);
    if (ZW_GECKO_CHIP_TYPE(my_chip_type)) {
      zwapi_nvr_get_value(offsetof(NVR_FLASH_STRUCT, aSecurityPrivateKey),
                          NVR_SECURITY_PRIVATE_KEY_SIZE,
                          buf);
    } else {
      zwapi_nvr_get_value(offsetof(NVR_FLASH_STRUCT, bRevision),
                          1,
                          &nvr_version);
      if ((0xff != nvr_version) && (nvr_version >= 2)) {
        zwapi_nvr_get_value(offsetof(NVR_FLASH_STRUCT, aSecurityPrivateKey),
                            NVR_SECURITY_PRIVATE_KEY_SIZE,
                            buf);
      } else {
        nvm_config_get(ecdh_priv_key, buf);
      }
    }
  }
}

void keystore_public_key_debug_print(void)
{
  uint8_t buf[32];
  keystore_public_key_read(buf);
}

void keystore_public_key_read(uint8_t *buf)
{
  uint8_t priv_key[32];

  keystore_private_key_read(priv_key);
  crypto_scalarmult_curve25519_base(buf, priv_key);
  memset(priv_key, 0, sizeof(priv_key));
}

bool keystore_network_key_read(uint8_t keyclass, uint8_t *buf)
{
  uint8_t assigned_keys = zwave_s2_keystore_get_assigned_keys();

  if (0 == (keyclass & assigned_keys)) {
    return false;
  }

  switch (keyclass) {
    case KEY_CLASS_S0:
      nvm_config_get(security_netkey, buf);
      break;
    case KEY_CLASS_S2_UNAUTHENTICATED:
      nvm_config_get(security2_key[0], buf);
      break;
    case KEY_CLASS_S2_AUTHENTICATED:
      nvm_config_get(security2_key[1], buf);
      break;
    case KEY_CLASS_S2_ACCESS:
      nvm_config_get(security2_key[2], buf);
      break;
    case KEY_CLASS_S2_AUTHENTICATED_LR:
      nvm_config_get(security2_lr_key[0], buf);
      break;
    case KEY_CLASS_S2_ACCESS_LR:
      nvm_config_get(security2_lr_key[1], buf);
      break;
    default:
      assert(0);
      return false;
  }

  return true;
}

void zwave_s2_log_security_keys(sl_log_level_t log_level)
{
  (void)log_level;  // Unused in Z-Wave build.
  uint8_t assigned_keys = zwave_s2_keystore_get_assigned_keys();
  for (uint8_t i = 0; i < 8; i++) {
    uint8_t current_keyclass = 1 << i;
    if (0 == (current_keyclass & assigned_keys)) {
      continue;
    }

    uint8_t current_key[NETWORK_KEY_SIZE] = {};

    switch (current_keyclass) {
      case KEY_CLASS_S0:
        nvm_config_get(security_netkey, current_key);
        sl_log(LOG_TAG,
               log_level,
               "Z-Wave S0 Network Key (class %x)",
               current_keyclass);
        break;
      case KEY_CLASS_S2_UNAUTHENTICATED:
        nvm_config_get(security2_key[0], current_key);
        sl_log(LOG_TAG,
               log_level,
               "Z-Wave S2 Unauthenticated Network Key (class 0x%x)",
               current_keyclass);
        break;
      case KEY_CLASS_S2_AUTHENTICATED:
        nvm_config_get(security2_key[1], current_key);
        sl_log(LOG_TAG,
               log_level,
               "Z-Wave S2 Authenticated Network Key (class 0x%x)",
               current_keyclass);
        break;
      case KEY_CLASS_S2_ACCESS:
        nvm_config_get(security2_key[2], current_key);
        sl_log(LOG_TAG,
               log_level,
               "Z-Wave S2 Access Control Network Key (class 0x%x)",
               current_keyclass);
        break;
      case KEY_CLASS_S2_AUTHENTICATED_LR:
        nvm_config_get(security2_lr_key[0], current_key);
        sl_log(LOG_TAG,
               log_level,
               "Z-Wave LR S2 Authenticated Network Key (class 0x%x)",
               current_keyclass);
        break;
      case KEY_CLASS_S2_ACCESS_LR:
        nvm_config_get(security2_lr_key[1], current_key);
        sl_log(LOG_TAG,
               log_level,
               "Z-Wave LR S2 Access Control Network Key (class 0x%x)",
               current_keyclass);
        break;
      default:
        sl_log(LOG_TAG,
               log_level,
               "<unknown> Network Key (class 0x%x)",
               current_keyclass);
        break;
    }
    sl_log_byte_arr(LOG_TAG, log_level, current_key, 16);
  }
}

bool keystore_network_key_write(uint8_t keyclass, const uint8_t *buf)
{
  uint8_t assigned_keys = 0;

  switch (keyclass) {
    case KEY_CLASS_S0:
      nvm_config_set(security_netkey, buf);
      s0_set_key(buf);
      break;
    case KEY_CLASS_S2_UNAUTHENTICATED:
      nvm_config_set(security2_key[0], buf);
      break;
    case KEY_CLASS_S2_AUTHENTICATED:
      nvm_config_set(security2_key[1], buf);
      break;
    case KEY_CLASS_S2_ACCESS:
      nvm_config_set(security2_key[2], buf);
      break;
    case KEY_CLASS_S2_AUTHENTICATED_LR:
      nvm_config_set(security2_lr_key[0], buf);
      break;
    case KEY_CLASS_S2_ACCESS_LR:
      nvm_config_set(security2_lr_key[1], buf);
      break;

    default:
      assert(0);
      return false;
  }
  assigned_keys = zwave_s2_keystore_get_assigned_keys();
  assigned_keys |= keyclass;
  nvm_config_set(assigned_keys, &assigned_keys);
  return true;
}

void zwave_s2_keystore_set_ecdh_key_mode(zwave_s2_keystore_ecdh_key_mode_t mode)
{
  ecdh_key_mode = mode;
}

uint8_t zwave_s2_keystore_get_assigned_keys()
{
  uint8_t assigned_keys = 0;

  nvm_config_get(assigned_keys, &assigned_keys);
  return assigned_keys;
}

void zwave_s2_keystore_reset_assigned_keys()
{
  uint8_t assigned_keys = 0;
  nvm_config_set(assigned_keys, &assigned_keys);
}

void zwave_s2_create_new_dynamic_ecdh_key()
{
  AES_CTR_DRBG_Generate(&s2_ctr_drbg, dynamic_ecdh_private_key);
  AES_CTR_DRBG_Generate(&s2_ctr_drbg, &dynamic_ecdh_private_key[16]);
}

static void zwave_s2_create_new_learn_mode_ecdh_key()
{
  zwave_s2_create_new_dynamic_ecdh_key();
  nvm_config_set(ecdh_priv_key, dynamic_ecdh_private_key);
  zwave_s2_create_new_dynamic_ecdh_key();
}

void zwave_s2_create_new_network_keys()
{
  uint8_t net_key[16] = {0};
  sl_log_info(LOG_TAG, "Creating new network keys\n");
  for (int c = 0; c < S2_NUM_KEY_CLASSES - 1; c++) {
    S2_get_hw_random(net_key, 16);
    keystore_network_key_write(1 << c, net_key);
  }

  // Also create a new S0 key:
  S2_get_hw_random(net_key, 16);
  keystore_network_key_write(KEY_CLASS_S0, net_key);

  memset(net_key, 0, sizeof(net_key));
}

void zwave_s2_keystore_init()
{
  uint32_t magic = 0;

  if (SL_STATUS_OK != nvm_config_get(magic, &magic)) {
    sl_log_error(LOG_TAG,
                 "Failed to read magic from zwapi_memory_get_buffer\n");
    assert(false);
  }
  magic = zpc_ntohl(magic);
  if (magic != NVM_MAGIC) {
    sl_log_warning(LOG_TAG,
                   "NVM magic check failed %08x != %08x, generating new keys\n",
                   magic,
                   NVM_MAGIC);
    zwave_s2_create_new_learn_mode_ecdh_key();
    zwave_s2_create_new_network_keys();

    magic = zpc_ntohl(NVM_MAGIC);
    nvm_config_set(magic, &magic);
  }
  zwave_s2_create_new_dynamic_ecdh_key();
}

void zwave_s2_keystore_get_dsk(zwave_s2_keystore_ecdh_key_mode_t mode,
                               zwave_dsk_t dsk)
{
  uint8_t pub_key[32];
  zwave_s2_keystore_ecdh_key_mode_t mode_save = ecdh_key_mode;
  zwave_s2_keystore_set_ecdh_key_mode(mode);
  keystore_public_key_read(pub_key);
  zwave_s2_keystore_set_ecdh_key_mode(mode_save);
  memcpy(dsk, pub_key, sizeof(zwave_dsk_t));
}
