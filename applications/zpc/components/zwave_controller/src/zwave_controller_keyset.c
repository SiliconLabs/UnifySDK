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

// Includes from this component
#include "zwave_controller_keyset.h"

// Generic includes
#include <stdlib.h>  // for NULL

// Includes from other components
#include "zwave_unid.h"
#include "zwave_network_management.h"
#include "zwave_tx_scheme_selector.h"

zwave_controller_encapsulation_scheme_t
  zwave_controller_get_highest_encapsulation(zwave_keyset_t keyset)
{
  if (keyset & ZWAVE_CONTROLLER_S2_ACCESS_KEY) {
    return ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS;
  } else if (keyset & ZWAVE_CONTROLLER_S2_AUTHENTICATED_KEY) {
    return ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_AUTHENTICATED;
  } else if (keyset & ZWAVE_CONTROLLER_S2_UNAUTHENTICATED_KEY) {
    return ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_UNAUTHENTICATED;
  } else if (keyset & ZWAVE_CONTROLLER_S0_KEY) {
    return ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0;
  } else {
    return ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  }
}

bool zwave_controller_encapsulation_scheme_greater_equal(
  zwave_controller_encapsulation_scheme_t k,
  zwave_controller_encapsulation_scheme_t v)
{
  zwave_keyset_t zpc_keyset;
  zwave_tx_scheme_get_zpc_granted_keys(&zpc_keyset);
  if (k == ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME) {
    k = zwave_controller_get_highest_encapsulation(zpc_keyset);
  }
  if (v == ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME) {
    v = zwave_controller_get_highest_encapsulation(zpc_keyset);
  }
  return k >= v;
}