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

/**
 * @defgroup zwave_helper_macros Z-Wave helper macros
 * @ingroup zwave_definitions
 * @brief Generic Z-Wave helper macros definitions
 *
 * Helper macros for Z-Wave related operations
 *
 * @{
 */

#ifndef ZWAVE_HELPER_MACROS_H
#define ZWAVE_HELPER_MACROS_H

/// Z-Wave API/tx send_data return codes

///< Transmission was successful.
#define TRANSMIT_COMPLETE_OK 0x00
///< The destination has not acknowledge the frame
#define TRANSMIT_COMPLETE_NO_ACK 0x01
///< Transmit error
#define TRANSMIT_COMPLETE_FAIL 0x02
///< Transmit error
#define TRANSMIT_ROUTING_NOT_IDLE 0x03
/// Assign route transmit complete but no routes was found
#define TRANSMIT_COMPLETE_NOROUTE 0x04
///< Verified delivery
#define TRANSMIT_COMPLETE_VERIFIED 0x05

// Helper macro that will indicate if a Z-Wave API transmission status is successful.
#define IS_TRANSMISSION_SUCCESSFUL(status) \
  ((status == TRANSMIT_COMPLETE_OK) || (status == TRANSMIT_COMPLETE_VERIFIED))

// Helper macro making a string out of a symbol
#define STR_CASE(x) \
  case x:           \
    return #x;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_HELPER_MACROS_H
/** @} end zwave_helper_macros */
