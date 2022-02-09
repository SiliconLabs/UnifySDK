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
 * @defgroup zwave_security_validation Z-Wave Security Validation
 * @ingroup zwave_controller
 * @brief Utility to verify if a frame should be discarded or kept based on its
 * security level.
 *
 * @{
 */

#ifndef ZWAVE_SECURITY_VALIDATION_H
#define ZWAVE_SECURITY_VALIDATION_H

// Includes from other components
#include "zwave_controller_connection_info.h"
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Verifies if a frame supported by us can be accepted based on the
 *        security it was received on.
 *
 * @param minimal_scheme     The minimal security scheme that the Command Class
 *                           handler has registered.
 * @param connection        Connection Info object to use for the remote node information.
 * @returns True if the frame is accepted and should be accepted. False otherwise.
 */
bool zwave_security_validation_is_security_valid_for_support(
  zwave_controller_encapsulation_scheme_t minimal_scheme,
  const zwave_controller_connection_info_t *connection);

/**
 * @brief Verifies if a frame supported by another node can be accepted based on
 *        the security it was received on.
 *
 * @param connection        Connection Info object to use for the remote node information.
 * @returns True if the frame is accepted and should be accepted. False otherwise.
 */
bool zwave_security_validation_is_security_valid_for_control(
  const zwave_controller_connection_info_t *connection);

/**
 * @brief Verifies if a the S2 Command Class has disappeared from the NIF, whereas
 * it was reported as supported previously.
 *
 * @param node_id             The NodeID for which the NIF is processed.
 * @param nif                 Raw byte array for the NIF.
 * @param nif_length          The length of the byte array.
 * @returns True if the frame is accepted and should go through. False otherwise.
 */
bool zwave_security_validation_is_s2_nif_downgrade_attack_detected(
  zwave_node_id_t node_id, const uint8_t *nif, uint8_t nif_length);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_SECURITY_VALIDATION_H
/** @} end zwave_security_validation */
