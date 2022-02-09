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
#include "zwave_security_validation.h"
#include "zwave_tx_scheme_selector.h"

//Includes from other components
#include "zwave_controller_utils.h"
#include "zwave_controller_keyset.h"
#include "zwave_controller_storage.h"
#include "ZW_classcmd.h"

#include "sl_log.h"
#define LOG_TAG "zwave_security_validation"

///////////////////////////////////////////////////////////////////////////////
// Public functions
///////////////////////////////////////////////////////////////////////////////
bool zwave_security_validation_is_security_valid_for_support(
  zwave_controller_encapsulation_scheme_t minimal_scheme,
  const zwave_controller_connection_info_t *connection)
{
  if (connection == NULL) {
    return false;
  }

  zwave_controller_encapsulation_scheme_t zpc_scheme
    = zpc_highest_security_class();

  // Minimum levels
  if ((minimal_scheme == ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS)
      || (minimal_scheme
          == ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_AUTHENTICATED)
      || (minimal_scheme
          == ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_UNAUTHENTICATED)
      || (minimal_scheme == ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0)) {
    if (!zwave_controller_encapsulation_scheme_greater_equal(zpc_scheme,
                                                             minimal_scheme)) {
      return false;
    }
  }

  // Received at our highest scheme: We like it :-)
  if (connection->encapsulation == zpc_scheme) {
    return true;
  }

  // Received at a lower level: we check that the CC minimal scheme allows it
  // (i.e. non-secure CC) and that it is still the highest key of the sender.
  if (minimal_scheme == ZWAVE_CONTROLLER_ENCAPSULATION_NONE) {
    zwave_keyset_t remote_node_keys;
    // If we can't find the keys of the remote in our attribute store,
    // do not accept the frame!
    if (SL_STATUS_OK
        != zwave_controller_storage_get_node_granted_keys(connection->remote.node_id,
                                                 &remote_node_keys)) {
      return false;
    }

    /// TODO: UIC-1102 Wait for the 2021D spec release to be in force for cert to re-activate this.
    //if (connection->encapsulation
    //    == zwave_controller_get_highest_encapsulation(remote_node_keys)) {
    // return true;
    //}
    return true;
  }

  // If we get here, we reject!
  return false;
}

bool zwave_security_validation_is_security_valid_for_control(
  const zwave_controller_connection_info_t *connection)
{
  if (connection == NULL) {
    return false;
  }

  // Here we do the following:
  // 1. If the node does not support S2. Accept! :-S
  if (false
      == zwave_controller_storage_is_node_s2_capable(
        connection->remote.node_id)) {
    return true;
  }

  // 2. If the node supports S2, only at the highest key.
  zwave_keyset_t remote_node_keys;
  if (SL_STATUS_OK
      != zwave_controller_storage_get_node_granted_keys(connection->remote.node_id,
                                               &remote_node_keys)) {
    return false;
  }
  if (connection->encapsulation
      == zwave_controller_get_highest_encapsulation(remote_node_keys)) {
    return true;
  }

  return false;
}

bool zwave_security_validation_is_s2_nif_downgrade_attack_detected(
  zwave_node_id_t node_id, const uint8_t *nif, uint8_t nif_length)
{
  if ((zwave_controller_storage_is_node_s2_capable(node_id) == true)
      && (false
          == is_command_class_in_supported_list(COMMAND_CLASS_SECURITY_2,
                                                nif,
                                                nif_length))) {
    sl_log_warning(
      LOG_TAG,
      "S2 NIF Downgrade attack detected for NodeID %d. Security 2 Command "
      "Class used to be supported but is not present anymore",
      node_id);
    return true;
  }
  return false;
}
