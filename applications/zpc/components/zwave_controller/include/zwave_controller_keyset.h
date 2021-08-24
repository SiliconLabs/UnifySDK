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
 * @defgroup zwave_controller_keyset Z-Wave Controller keyset utility
 * @ingroup zwave_controller
 * @brief Small utility allowing to process keyset structures.
 *
 *
 * This utility is used to retrieve the list of keys that is granted to
 * the ZPC or any node in the network. It can also convert a keyset to
 * the corresponding highest encapsulation scheme that can be used for
 * a transmission.
 *
 * @{
 */
#ifndef ZWAVE_CONTROLLER_KEYSET_H
#define ZWAVE_CONTROLLER_KEYSET_H
#include "zwave_controller_connection_info.h"

#ifdef __cplusplus
extern "C" {
#endif


#define ZWAVE_CONTROLLER_S0_KEY                 0x80
#define ZWAVE_CONTROLLER_S2_UNAUTHENTICATED_KEY 0x01
#define ZWAVE_CONTROLLER_S2_AUTHENTICATED_KEY   0x02
#define ZWAVE_CONTROLLER_S2_ACCESS_KEY          0x04

typedef uint8_t zwave_keyset_t;

/**
 * @brief Get the hight encapsulation supported by a given key set.
 *
 * @param keyset @ref zwave_keyset_t representing the set of
 *                granted keys to a node
 * @returns zwave_controller_encapsulation_scheme_t value representing
 *          the highest granted key. Note that
 *          @ref ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME will not
 *          be returned.
 */
zwave_controller_encapsulation_scheme_t
  zwave_controller_get_highest_encapsulation(zwave_keyset_t keyset);

/**
 * @brief Function to tell whether scheme k is greater or equal to scheme v.
 *
 * @param k The first @ref zwave_controller_encapsulation_scheme_t variable.
 * @param v The second @ref zwave_controller_encapsulation_scheme_t variable.
 *
 * @return bool True if k is greater or equal to v; otherwise false
 */
bool zwave_controller_encapsulation_scheme_greater_equal(
  zwave_controller_encapsulation_scheme_t k,
  zwave_controller_encapsulation_scheme_t v);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_CONTROLLER_KEYSET_H

/** @} end zwave_controller_keyset */
