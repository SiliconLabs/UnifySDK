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
 * @defgroup zwave_generic_types Z-Wave Generic types
 * @ingroup zwave_definitions
 * @brief Generic Z-Wave Type definitions
 *
 * Module containing generic type definitions for Z-Wave.
 *
 * @{
 */

#ifndef ZWAVE_GENERIC_TYPES_H
#define ZWAVE_GENERIC_TYPES_H

#include <stdint.h>

typedef uint16_t zwave_command_class_t;
typedef uint8_t zwave_command_t;

/**
 * @brief Z-Wave Operating Modes (AL, FL or NL).
 * Refer to @ref zwave_nwk_spec for details.
 */
typedef enum zwave_operating_mode {
  /// Node is Always Listening (AL)
  ///
  OPERATING_MODE_AL,
  /// Node is Frequently Listening (FL)
  ///
  OPERATING_MODE_FL,
  /// Node is Non-Listening (NL)
  ///
  OPERATING_MODE_NL,
  /// We do not know the Operating mode of the node.
  ///
  OPERATING_MODE_UNKNOWN
} zwave_operating_mode_t;

/**
 * @brief Z-Wave Inclusion Protocol (Z-Wave / Z-Wave Long Range).
 */
typedef enum zwave_protocol {
  /// Node is operating on the Z-Wave PHY/MAC/NWK
  ///
  PROTOCOL_ZWAVE,
  /// Node is operating on the Z-Wave Long Range PHY/MAC/NWK
  ///
  PROTOCOL_ZWAVE_LONG_RANGE,
  /// We do not know the protocol of that node.
  ///
  PROTOCOL_UNKNOWN
} zwave_protocol_t;

/**
 * @brief Z-Wave Plus Role Type
 */
typedef enum zwave_role_type {
  /// Central Static Controller
  ROLE_TYPE_CSC = 0x00,
  /// Sub Static Controller
  ROLE_TYPE_SSC = 0x01,
  /// Portable Controller
  ROLE_TYPE_PC = 0x02,
  /// Reporting Portable Controller
  ROLE_TYPE_RPC = 0x03,
  /// Portable End Node
  ROLE_TYPE_PEN = 0x04,
  /// Always On End Node
  ROLE_TYPE_AOEN = 0x05,
  /// Reporting Sleeping End Node
  ROLE_TYPE_RSEN = 0x06,
  /// Listening Sleeping End Node
  ROLE_TYPE_LSEN = 0x07,
  /// Network Aware End Node
  ROLE_TYPE_NAEN = 0x08,
  /// Unknown role type.
  ROLE_TYPE_UNKNOWN = 0xFF,
} zwave_role_type_t;

#endif  //ZWAVE_GENERIC_TYPES_H
/** @} end zwave_generic_types */
